// GUI-only retained artwork. Scene data is read once; no control writes or audio work.
#include "display.h"
#include <opxy/track_control.hpp>
#include <opxy/morph_hud_snapshot.hpp>
#include "legacy_aux.hpp"
#include "stock_plot.hpp"
namespace morphcapture {
namespace d=opxy::draw;
using opxy::u32;using opxy::i32;
inline void display_reset(DisplayState& s){s.serial=0;s.track=0xffffffffu;for(u32 k=0;k<4;++k)s.flash[k]=0;}
inline void node(Frame& f,u32 n,i32 x,i32 y,i32 scale=1024,u32 color=0,d::Paint paint=d::keep){d::place(f,n,x*1024,y*1024,scale,color,paint);}
inline void txt(Frame& f,u32 n,i32 x,i32 y,const char* s,i32 scale=1024){d::text(f,n,x*1024,y*1024,scale,s);}
inline i32 norm(i32 v){return v<0?-1:static_cast<i32>(((static_cast<u32>(v)>>11)*99u+0x7ffffu)/0xfffffu);}
inline void number(char* s,i32 v){if(v<0){s[0]='-';s[1]='-';s[2]=0;return;}v=d::clamp(v,0,99);s[0]=static_cast<char>('0'+v/10);s[1]=static_cast<char>('0'+v%10);s[2]=0;}
inline void row_text(Frame& f,u32 slot,i32 x,i32 y,const i32* values){char line[24]={};for(u32 k=0;k<4;++k){number(line+k*5,norm(values[k]));if(k<3){line[k*5+2]=' ';line[k*5+3]=' ';line[k*5+4]=' ';}}txt(f,slot,x,y,line);}
inline void symbol(Frame& f,u32 copy,u32 kind,i32 x,i32 y,i32 scale=1024){node(f,ui::symbol0+copy*3+kind,x,y,scale);}
inline void sweep(Frame& f,i32 travel,bool live,bool vertical){
 travel=d::clamp(travel,-64,64);const i32 t=travel<0?-travel:travel;
 const i32 ease=t*t*(192-2*t)/256; // Q10 smoothstep at all 129 real positions.
 const i32 dist=69317+(184320-69317)*ease/1024;
 const i32 scale=vertical?614:1024,cx=vertical?419:240,cy=vertical?110:147;
 const i32 half=dist*scale/2048;
 d::place(f,ui::circleA,cx*1024-(vertical?0:half),cy*1024-(vertical?half:0),scale,live?0:1,d::fill_and_stroke);
 d::place(f,ui::circleB,cx*1024+(vertical?0:half),cy*1024+(vertical?half:0),scale,live?0:1,d::stroke);
 node(f,ui::masks[t],cx,cy,scale);
 if(vertical)d::turn(f,ui::masks[t],d::rotation(900));
 node(f,ui::pixels[t],cx,cy,scale,live?0:1,d::fill_and_stroke);
 if(vertical)d::turn(f,ui::pixels[t],d::rotation(900));
 if(!live){f.nodes[ui::stripes]=f.nodes[ui::circleA];d::shade(f,ui::stripes,5,d::stroke);}
 if(!t){const u32 cap=vertical?ui::capsuleHUD:ui::capsuleAux;node(f,cap,cx,cy,scale,live?0:1,d::stroke);if(vertical)d::turn(f,cap,d::rotation(900));}
 const i32 edge=(dist/2+61440)*ease/1024*scale/1024*(travel<0?-1:1);
 d::place(f,ui::square,cx*1024+(vertical?0:edge),cy*1024+(vertical?edge:0),scale,live?1:3,d::fill_and_stroke);
}
inline void init_frame(Frame& f){
 for(u32 n=0;n<Frame::node_count;++n)d::hide(f,n);
 for(u32 n=0;n<32;++n)txt(f,n,640,320,"");
 for(u32 k=0;k<4;++k){d::header(f,k," ");f.paths[k].key=1;for(u32 n=0;n<64;++n)f.paths[k].points[n]={0,0};}
 node(f,ui::cover,0,0);
}
inline void blink(DisplayState& s,const d::Input& in){
 const u32 serial=static_cast<u32>(opxy::control::from_control(9u));
 if(s.track!=in.track){s.serial=serial;s.track=in.track;for(u32 k=0;k<4;++k)s.flash[k]=0;}
 for(u32 k=0;k<4;++k){s.flash[k]=d::clamp(s.flash[k]-d::clamp(in.elapsed,0,1024),0,410);if(((serial^s.serial)>>(8*k))&255u)s.flash[k]=410;}
 s.serial=serial;
}
inline u32 icon_shade(const DisplayState& s,u32 k){return (s.flash[k]>307 || (s.flash[k]>102 && s.flash[k]<=205))?0u:6u;}
inline void aux_m2(Frame& f){
 morph_hud::Frame old={};for(u32 n=8;n<63;++n)d::hide(old,n);legacy_ui::aux_face(old);
 for(u32 n=0;n<63;++n)if(ui::legacy_nodes[n]<Frame::node_count)f.nodes[ui::legacy_nodes[n]]=old.nodes[n];
 d::hide(f,ui::cover);
 // The exact original routing geometry and text sizes, translated into the new slots.
 const u32 sizes[9]={12,12,12,12,14,14,20,26,10};
 for(u32 k=0;k<9;++k){f.texts[k]=old.texts[18+k];f.texts[k].scale=old.texts[18+k].scale*static_cast<i32>(sizes[k])/15;}
 for(u32 k=0;k<4;++k)d::header(f,k,old.header[k]);
}
inline void aux(Frame& f,const DisplayState& s){
 const u32 page=static_cast<u32>(opxy::control::from_control(4u));
 if(page==2){aux_m2(f);return;}
 for(u32 k=0;k<4;++k)node(f,ui::header0+k,120*static_cast<i32>(k),0);
 const u32 mode=static_cast<u32>(opxy::control::from_control(3u));
 if(page!=3){
  node(f,ui::symbol0,60,49,2048,icon_shade(s,0),d::fill_and_stroke);
  node(f,ui::symbol1,180,49,2048,icon_shade(s,1),d::stroke);
  node(f,ui::symbol2,300,49,2048,icon_shade(s,2)==0?1u:5u,d::fill_and_stroke);
  node(f,ui::modeMorph,420,49,1100,icon_shade(s,3),d::fill_and_stroke);
  node(f,ui::modeMorphB,420,49,1100,icon_shade(s,3),d::stroke);node(f,ui::modeMorphPixel,420,49,1100,icon_shade(s,3),d::fill_and_stroke);
  node(f,ui::modeMorphRed,420,49,1100,icon_shade(s,3)==0?1:5,d::fill_and_stroke);node(f,ui::modeMorphSquare,420,49,1100);
  sweep(f,opxy::control::from_control(2u),(mode&16u)!=0,false);return;
 }
 const u32 pad=static_cast<u32>(opxy::control::from_control(7u));
 const u32 preset=static_cast<u32>(opxy::control::from_control(8u));
 node(f,ui::pill,60,44);node(f,(pad&1u)?ui::modeMorph:ui::notes,60,44,1024,icon_shade(s,0),d::fill_and_stroke);
 if(pad&1u){node(f,ui::modeMorphB,60,44,1024,icon_shade(s,0),d::stroke);node(f,ui::modeMorphPixel,60,44,1024,icon_shade(s,0),d::fill_and_stroke);node(f,ui::modeMorphSquare,60,44);}
 node(f,ui::arrow,180,44,1024,icon_shade(s,1),d::stroke);
 node(f,ui::pair,300,44,1024,icon_shade(s,2),d::fill_and_stroke);node(f,ui::pairB,300,44,1024,icon_shade(s,2),d::stroke);
 node(f,ui::bookmark,420,44,1024,icon_shade(s,3),d::stroke);
 node(f,ui::curve0+((pad>>16)&3u)%3u,86,132);
 const i32 sens=static_cast<i32>((pad>>8)&255u)-90;
 node(f,ui::vector,91,176,64+(sens<0?-sens:sens)*16,sens<0?1:0,d::stroke);
 if(sens<0)d::turn(f,ui::vector,d::rotation(1800));
 if(sens==101)node(f,ui::infinity,155,201);
 const u32 selected=(preset>>4)&15u;
 for(u32 k=0;k<16;++k){const i32 x=255+static_cast<i32>(k%4)*54,y=76+static_cast<i32>(k/4)*32;
  // Cell fills stay authored black; only one movable node supplies selection.
  node(f,ui::slot0+k,x,y,1024,mode&16u?1:0,d::stroke);
  if(k==selected)node(f,ui::slotSelection,x,y,1024,mode&16u?1:0,d::fill_and_stroke);
  char text[3];number(text,static_cast<i32>(k+1));txt(f,k==selected?24:k,x+13,y+19,text,900);
 }
 if(mode&16u)node(f,ui::gridStripe,255,76);
 const u32 status=(preset>>8)&255u;
 const char* msg=status==1?"Saved":status==2?"Loaded":status==3?"M1 preserved":status==5?"BYPASS required":status==6?"Empty or damaged":status==7?"File/context error":status==8?"Capture A/B first":"";
 txt(f,23,252,215,msg,720);
}
// Normalized stock ADSR Bezier templates sampled at 32 vertices and retraced.
// Release is mirrored, exactly as the stock envelope control contract specifies.
inline i32 mix(i32 a,i32 b,i32 t){return a+(b-a)*t/1024;}
inline i32 bez(i32 a,i32 b,i32 c,i32 z,i32 t){return mix(mix(mix(a,b,t),mix(b,c,t),t),mix(mix(b,c,t),mix(c,z,t),t),t);}
// Coordinates remain Q10 through Bezier evaluation and placement.
// Never round a curve to whole pixels before scaling it.
inline i32 extent(i32 v,i32 range){return (v>>16)*range/32767;}
inline void envelope(Frame& f,u32 p,const i32* v,i32 x,i32 y,bool edit){
 for(u32 i=0;i<32;++i){
  f.paths[p].points[i]=stock_plot::envelope(v,i);
  f.paths[p].points[63-i]=f.paths[p].points[i];
 }
 u32 hash=2166136261u;for(u32 i=0;i<4;++i)hash=(hash^static_cast<u32>(v[i]>>16))*16777619u;f.paths[p].key=hash|0x80000000u;
 d::place(f,p,x*1024,y*1024+256,1792,edit?1:0,d::stroke);
}
inline void hud_m1(Frame& f,const hud::Snapshot& s){
 const u32 slots[26]={0,1,2,3,4,5,6,7,8,9,10,11,13,15,16,17,18,19,20,21,22,23,32,33,34,35};
 for(u32 bank=0;bank<(s.m1.dual?2u:1u);++bank){
  i32 values[3][4];for(u32 k=0;k<4;++k){u32 at=26,physical=bank?s.m1.alternate[k]:s.m1.primary[k];for(u32 i=0;i<26;++i)if(slots[i]==physical)at=i;
   values[0][k]=at<26&&s.captured_a?s.a[at]:-1;values[1][k]=at<26&&s.start_ready?s.s[at]:-1;values[2][k]=at<26&&s.captured_b?s.b[at]:-1;}
  const i32 y=12+static_cast<i32>(bank)*102;bool focus=s.bank_focus==bank;
  if(focus)node(f,ui::focus0+bank,8,y);
  txt(f,focus?24+bank:10+bank,12,y+16,bank?"Settings 2":"Settings 1",850);
  for(u32 row=0;row<3;++row){const i32 yy=y+static_cast<i32>(row)*27;
   node(f,ui::row0+bank*3+row,100,yy,730,row==1?0:4,d::fill_and_stroke);
   symbol(f,bank,row==0?0:row==1?2:1,111,yy+10,750);
   const bool edit=(row==0&&s.edit_side==0)||(row==2&&s.edit_side==1);
   row_text(f,row==1?26+bank:edit?16+bank*2+(row==2):bank*2+(row==2),128,yy+15,values[row]);
  }
 }
}
template<unsigned N>inline void stock_icon(Frame& f,const u32 (&ids)[N],i32 x,i32 y){for(u32 i=0;i<N;++i)node(f,ids[i],x,y,850);}
inline void shift(Frame& f,const hud::Snapshot& s){
 node(f,ui::focus0,8,12);txt(f,24,14,28,"SHIFT",900);symbol(f,0,0,263,30);symbol(f,1,1,321,30);
 for(u32 k=0;k<4;++k){i32 y=52+static_cast<i32>(k)*35;node(f,ui::row0+k,8,y,1024,4,d::fill_and_stroke);
  if(s.module==2){switch(k){case 0:stock_icon(f,ui::icons4,39,y+16);break;case 1:stock_icon(f,ui::icons5,39,y+16);break;case 2:stock_icon(f,ui::icons6,39,y+16);break;default:stock_icon(f,ui::icons7,39,y+16);}}
  else {switch(k){case 0:stock_icon(f,ui::icons0,39,y+16);break;case 1:stock_icon(f,ui::icons1,39,y+16);break;case 2:stock_icon(f,ui::icons2,39,y+16);break;default:stock_icon(f,ui::icons3,39,y+16);}}
  const u32 at=s.module==2?18+k:k==1?12:k==3?13:26;
  char text[3];number(text,at<26?norm(s.current[at]):-1);txt(f,k,184,y+22,text);
  if(s.module==1 && k==0){const char* pm[3]={"poly","mono","legato"};const u32 choice=s.playmode<0?0u:static_cast<u32>(s.playmode>>8)*3u>>23;txt(f,k,130,y+22,pm[choice<3?choice:2],900);}
  if(s.module==1 && k==2){const char* br[6]={"off","1 semitone","2 semitones","4 semitones","7 semitones","octave"};const u32 choice=s.bendrange<0?0u:static_cast<u32>(s.bendrange>>8)*6u>>23;txt(f,k,120,y+22,br[choice<6?choice:5],700);}
  number(text,at<26&&s.captured_a?norm(s.a[at]):-1);txt(f,s.edit_side?4+k:16+k,254,y+22,text);
  number(text,at<26&&s.captured_b?norm(s.b[at]):-1);txt(f,s.edit_side?20+k:8+k,312,y+22,text);
 }
}
// Stock-resource wireframe plots; values stay next to their controls instead
// of occupying prose rows across the graph. All curve coordinates are Q10.
inline void filter(Frame& f,const hud::Snapshot& s,u32 side){
 const i32 row=static_cast<i32>(side)*104,cy=60+row;
 const i32* v=(side?s.b:s.a)+14;const bool edit=s.edit_side==side;
 const u32 body=side*2,edge=body+1;
 node(f,ui::hudHorizontal,0,0);
 const char* name=s.filter_type==17?"high-pass":s.filter_type==9?"z lowpass":s.filter_type==10?"svf":s.filter_type==16?"ladder":s.filter_type==18?"dynamic":"filter";
 txt(f,side,32,18+row,name,819);symbol(f,side,side?1:0,18,15+row,850);
 if(!(side?s.captured_b:s.captured_a)){txt(f,2+side,62,60+row,"Scene unavailable",819);return;}
 const u32 axis=side?ui::filterAxis1_0:ui::filterAxis0_0;
 for(u32 k=0;k<4;++k)node(f,axis+k,62,94+row);
 const char* labels[5]={"50","1k","2k","5k","20kHz"};const i32 xs[5]={62,139,193,241,298};
 for(u32 k=0;k<5;++k)txt(f,6+side*5+k,xs[k],105+row,labels[k],410);
 // Scale the actual stock LP/HP cubic outlines into the approved plot area.
 for(u32 i=0;i<32;++i){
  d::Point q=stock_plot::filter_response(v[0],s.filter_type==17,i,row);
  i32 x=(q.x-190*1024)/2,y=(q.y-cy*1024)/2;
  if(s.filter_type==18){
   // Dynamic has independent HP (K1) and LP (K4) cutoffs, not
   // centre/width. This schematic combines their attenuated regions.
   x=-65536+static_cast<i32>(i)*131072/31;
   const i32 low=-65536+stock_plot::amount(v[0],131072);
   const i32 high=-65536+stock_plot::amount(v[3],131072);
   const i32 hp=d::clamp((low-x+10240)/20,0,1024);
   const i32 lp=d::clamp((x-high+10240)/20,0,1024);
   const i32 t=hp>lp?hp:lp;
   y=-16384+32*(t*t/1024*(3072-2*t)/1024);
  }
  f.paths[edge].points[i]={x,y};f.paths[edge].points[63-i]={x,y};
 }
 if(s.filter_type!=18){
  // Clip the sampled stock curve, preserving intersections rather than
  // projecting offscreen points into a spurious vertical border line.
  const i32 left=-65536,right=(stock_plot::filter_x(461824)-190*1024)/2;
  i32 yl=f.paths[edge].points[0].y,yr=f.paths[edge].points[31].y;
  for(u32 i=1;i<32;++i){const d::Point a=f.paths[edge].points[i-1],b=f.paths[edge].points[i];
   if(b.x>a.x){
    if(a.x<=left && b.x>=left)yl=a.y+(b.y-a.y)*(left-a.x)/(b.x-a.x);
    if(a.x<=right && b.x>=right)yr=a.y+(b.y-a.y)*(right-a.x)/(b.x-a.x);
   }
  }
  for(u32 i=0;i<32;++i){d::Point& q=f.paths[edge].points[i];
   if(q.x<left){q={left,yl};}
   if(q.x>right){q={right,yr};}
   f.paths[edge].points[63-i]=q;
  }
 }
 u32 key=2166136261u;key=(key^static_cast<u32>(v[0]>>16))*16777619u;
 key=(key^static_cast<u32>((s.filter_type==18?v[3]:0)>>16))*16777619u;key=(key^s.filter_type)*16777619u;key&=0x7fffffffu;if(!key)key=1u;
 f.paths[body].key=key;f.paths[edge].key=key;
 // The closed body remains hidden: only the response edge is wanted.
 node(f,edge,190,cy,2048,edit?1:3,d::stroke);
 // Stock-style floating cutoff and resonance values, plus envelope/key icons.
 const u32 text=edit?16+side*4:2;
 char value[3];
 if(s.filter_type==18){
  // Four visual controls follow encoder order: low cut, comp, dist, high cut.
  // No resonance readout exists for this model.
  const i32 low=62+stock_plot::amount(v[0],256);
  const i32 high=62+stock_plot::amount(v[3],256);
  number(value,norm(v[0]));txt(f,text,d::clamp(low-7,62,300),38+row,value,819);
  number(value,norm(v[3]));txt(f,text+3,d::clamp(high-7,62,300),87+row,value,819);
  for(u32 k=0;k<2;++k){
   char label[5]={k?'D':'C',' ',0,0,0};number(label+2,norm(v[1+k]));
   txt(f,text+1+k,10,51+row+static_cast<i32>(k)*30,label,750);
   node(f,ui::dynamicBar0+side*2+k,10,57+row+static_cast<i32>(k)*30,
        128+(v[1+k]>>23),edit?1:3,d::fill_and_stroke);
  }
  return;
 }
 // cutoff_box's authored text anchor (-41.1501,106.2457), translated by
 // cutoff*375 stock pixels. Keep the tiny value readable at the crop edge.
 const i32 cutoff_x=d::clamp(stock_plot::filter_x(-42138+stock_plot::amount(v[0],384000))/1024,66,289);
 node(f,side?ui::filterValueBox1:ui::filterValueBox0,cutoff_x-3,43+row);
 number(value,norm(v[0]));txt(f,text,cutoff_x,54+row,value,819);
 number(value,norm(v[1]));
 const d::Point q=stock_plot::resonance_label(v,row);
 d::text(f,text+1,q.x,q.y,819,value);
 node(f,side?ui::filterEnv1:ui::filterEnv0,10,43+row);
 node(f,side?ui::filterKey1:ui::filterKey0,10,72+row);
 number(value,norm(v[2]));txt(f,text+2,35,51+row,value,750);
 number(value,norm(v[3]));txt(f,text+3,35,81+row,value,750);
}
inline void display(DisplayState& state,const d::Input& input,Frame& f){
 init_frame(f);blink(state,input);
 if(input.track==11){aux(f,state);return;}
 hud::Snapshot s={};if(input.track>=8 || !sdk_morph_hud_snapshot_read(input.track,&s)||s.abi!=hud::kSnapshotABI){txt(f,0,12,28,"Morph unavailable");return;}
 node(f,ui::divider,0,0);sweep(f,s.travel,s.mode!=0,true);
 if(s.shifted&&s.module!=0){shift(f,s);return;}
 if(s.module==0){hud_m1(f,s);return;}
 if(s.module==2){filter(f,s,0);filter(f,s,1);return;}
 node(f,ui::hudHorizontal,0,0);node(f,ui::hudVertical,0,0);
 for(u32 side=0;side<2;++side)for(u32 bank=0;bank<2;++bank){
  const i32 x=10+static_cast<i32>(bank)*177,y=10+static_cast<i32>(side)*104;
  const u32 p=side*2+bank;const bool focus=s.bank_focus==bank;
  if(focus)node(f,ui::envelopeTitle0+p*2,x,y);
  txt(f,focus?24+p:p,x+4,y+14,bank?"FILTER":"AMP",819);
  node(f,ui::envelopeScene0+p*2,x+(bank?51:37)+12,y+10);
  if(side?s.captured_b:s.captured_a)envelope(f,p,(side?s.b:s.a)+(bank?22:8),x+80,y+62,s.edit_side==side && focus);
 }
}
}
