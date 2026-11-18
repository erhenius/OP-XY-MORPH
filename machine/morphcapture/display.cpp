// GUI-only retained artwork. Scene data is read once; no control writes or audio work.
#include "display.h"
#include <opxy/track_control.hpp>
#include <opxy/morph_hud_snapshot.hpp>
#include "legacy_aux.hpp"
#include "stock_plot.hpp"
#include "trine_sweep.hpp"
namespace morphcapture {
namespace d=opxy::draw;
using opxy::u32;using opxy::i32;
inline void display_reset(DisplayState& s){s.serial=0;s.track=0xffffffffu;for(u32 k=0;k<4;++k){s.flash[k]=0;s.curves[k].kind=0;}}
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
inline void aux(Frame& f,DisplayState& s){
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
  const i32 travel=opxy::control::from_control(2u);sweep(f,travel,(mode&16u)!=0,false);
  return;
 }
 const u32 pad=static_cast<u32>(opxy::control::from_control(7u));
 const u32 preset=static_cast<u32>(opxy::control::from_control(8u));
 node(f,ui::pill,60,44);node(f,(pad&1u)?ui::modeMorph:ui::notes,60,44,1024,icon_shade(s,0),d::fill_and_stroke);
 if(pad&1u){node(f,ui::modeMorphB,60,44,1024,icon_shade(s,0),d::stroke);node(f,ui::modeMorphPixel,60,44,1024,icon_shade(s,0),d::fill_and_stroke);node(f,ui::modeMorphSquare,60,44);}
 node(f,ui::arrow,180,44,1024,icon_shade(s,1),d::stroke);
 node(f,ui::pair,300,44,1024,icon_shade(s,2),d::fill_and_stroke);node(f,ui::pairB,300,44,1024,icon_shade(s,2),d::stroke);
 node(f,ui::bookmark,420,44,1024,icon_shade(s,3),d::stroke);
 node(f,ui::padLeft,136,36,492,1,d::stroke);
 node(f,ui::padRight,224,36,492,0,d::stroke);
 node(f,ui::gainUp,256,36,492,0,d::stroke);
 node(f,ui::gainDown,344,36,492,1,d::stroke);
 // The authored arrow points 39.8 degrees above the horizontal.
 d::turn(f,ui::padLeft,d::rotation(2198));
 d::turn(f,ui::padRight,d::rotation(398));
 d::turn(f,ui::gainUp,d::rotation(-502));
 d::turn(f,ui::gainDown,d::rotation(1298));
 node(f,ui::curve0+((pad>>17)&3u)%3u,138,129,896);
 const i32 sens=static_cast<i32>((pad>>8)&511u)-90;
 const i32 pad_scale=sens<0?64-sens*16:64+d::clamp(sens,0,200)*11;
 node(f,ui::vector,91,176,pad_scale,sens<0?1:0,d::stroke);
 if(sens<0)d::turn(f,ui::vector,d::rotation(1800));
 if(sens==201)node(f,ui::infinity,155,201);
 const i32 percent=static_cast<i32>((pad>>20)&127u)*5-95;
 if(percent){
  const i32 scale=percent<0?96-percent*6:64+percent*5;
  node(f,ui::hudVector,91,176,scale,percent<0?1:0,d::stroke);
  d::turn(f,ui::hudVector,d::rotation(percent<0?900:-900));
 }
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
// Compare full inputs, not the lossy renderer key. Cache only curve geometry;
// placement, highlight colours, labels and snapshot reads remain live.
inline bool curve_cached(const CurveCache* c,u32 kind,const i32* values){
 if(!c || c->kind!=kind)return false;
 for(u32 i=0;i<4;++i)if(c->values[i]!=values[i])return false;
 return true;
}
inline void curve_save(CurveCache* c,u32 kind,const i32* values,const Frame::path_type& path){
 if(!c)return;
 c->path=path;for(u32 i=0;i<4;++i)c->values[i]=values[i];c->kind=kind;
}
// Turn a retraced centreline into a finite-width, filled ribbon. Flat end
// faces replace the degenerate 180-degree stroke joins at the visible ends.
// All geometry stays Q10; no floating point, allocation or SDK changes.
inline opxy::u32 root_floor(opxy::u32 n){
 u32 r=0,bit=1u<<30;
 while(bit>n)bit>>=2;
 while(bit){if(n>=r+bit){n-=r+bit;r=(r>>1)+bit;}else r>>=1;bit>>=2;}
 return r;
}
inline void curve_ribbon(Frame& f,u32 p,bool sustain=false){
 d::Point center[32];for(u32 i=0;i<32;++i)center[i]=f.paths[p].points[i];
 for(u32 i=0;i<32;++i){
  // ADSR vertices 19..22 bracket the horizontal hold (including the
  // duplicated stage junctions). Use its exact normal at both ends;
  // neighbouring curved tangents would taper the entire hold segment.
  if(sustain && i>=19 && i<=22){
   f.paths[p].points[i]={center[i].x,center[i].y+320};
   f.paths[p].points[63-i]={center[i].x,center[i].y-320};
   continue;
  }
  u32 a=i,b=i;
  while(a && center[a].x==center[i].x && center[a].y==center[i].y)--a;
  while(b<31 && center[b].x==center[i].x && center[b].y==center[i].y)++b;
  i32 dx=center[b].x-center[a].x,dy=center[b].y-center[a].y;
  // Bound squared length and multiplication on the 32-bit target.
  while(dx>16383 || dx< -16383 || dy>16383 || dy< -16383){dx/=2;dy/=2;}
  while((dx || dy) && dx<4096 && dx> -4096 && dy<4096 && dy> -4096){dx*=2;dy*=2;}
  const i32 length=static_cast<i32>(root_floor(static_cast<u32>(dx*dx+dy*dy)));
  const i32 nx=length?-dy*320/length:0,ny=length?dx*320/length:320;
  f.paths[p].points[i]={d::clamp(center[i].x+nx,-65536,65536),d::clamp(center[i].y+ny,-65536,65536)};
  f.paths[p].points[63-i]={d::clamp(center[i].x-nx,-65536,65536),d::clamp(center[i].y-ny,-65536,65536)};
 }
}
// Offset curves can cross at a tight reversal (zero attack) or near a
// shallow sustain/release join. Untangle the boundary without moving vertices.
// Bounding boxes reject almost all pairs before the exact orientation tests.
// ADSR ribbon bounds are within x +/-44 and y +/-20 local pixels.
// Thus each absolute orientation product fits in unsigned 32 bits even
// when their signed difference would not. Compare sign/magnitude instead
// of constructing that difference with costly 64-bit multiplication.
static_assert(90112ull*40960ull<=0xffffffffull,"ADSR orientation products must fit u32");
inline i32 edge_side(const d::Point& a,const d::Point& b,const d::Point& c){
 const i32 dx=b.x-a.x,dy=b.y-a.y,cx=c.x-a.x,cy=c.y-a.y;
 const u32 first=static_cast<u32>(dx<0?-dx:dx)*static_cast<u32>(cy<0?-cy:cy);
 const u32 second=static_cast<u32>(dy<0?-dy:dy)*static_cast<u32>(cx<0?-cx:cx);
 const bool negative_first=first && ((dx<0)!=(cy<0));
 const bool negative_second=second && ((dy<0)!=(cx<0));
 if(negative_first!=negative_second)return negative_first?-1:1;
 if(first==second)return 0;
 return ((first>second)!=negative_first)?1:-1;
}
inline bool opposite(i32 a,i32 b){return (a<0 && b>0)||(a>0 && b<0);}
inline bool crossed(const d::Point& a,const d::Point& b,const d::Point& c,const d::Point& e){
 const i32 ax0=a.x<b.x?a.x:b.x,ax1=a.x>b.x?a.x:b.x;
 const i32 ay0=a.y<b.y?a.y:b.y,ay1=a.y>b.y?a.y:b.y;
 const i32 bx0=c.x<e.x?c.x:e.x,bx1=c.x>e.x?c.x:e.x;
 const i32 by0=c.y<e.y?c.y:e.y,by1=c.y>e.y?c.y:e.y;
 if(ax1<=bx0 || bx1<=ax0 || ay1<=by0 || by1<=ay0)return false;
 return opposite(edge_side(a,b,c),edge_side(a,b,e)) && opposite(edge_side(c,e,a),edge_side(c,e,b));
}
inline u32 untangle_envelope(Frame& f,u32 p){
 d::Point* q=f.paths[p].points;u32 edits=0;
 // Sweep edges in left-to-right order; disjoint x ranges cannot cross.
 // This avoids scanning all 2016 edge pairs after each local repair.
 unsigned char order[64];i32 left[64];
 for(u32 pass=0;pass<64;++pass){
  u32 count=0;
  for(u32 i=0;i<64;++i){
   const d::Point a=q[i],b=q[(i+1)%64];
   if(a.x==b.x && a.y==b.y)continue;
   left[i]=a.x<b.x?a.x:b.x;
   u32 at=count;while(at && left[order[at-1]]>left[i]){order[at]=order[at-1];--at;}
   order[at]=static_cast<unsigned char>(i);++count;
  }
  bool changed=false;
  for(u32 a=0;a<count && !changed;++a){
   const u32 i=order[a];const i32 right=q[i].x>q[(i+1)%64].x?q[i].x:q[(i+1)%64].x;
   for(u32 b=a+1;b<count;++b){
    const u32 j=order[b];if(left[j]>right)break;
    const u32 lo=i<j?i:j,hi=i>j?i:j;
    if(hi==lo+1 || (lo==0 && hi==63))continue;
    if(!crossed(q[i],q[(i+1)%64],q[j],q[(j+1)%64]))continue;
    for(u32 x=lo+1,y=hi;x<y;++x,--y){const d::Point tmp=q[x];q[x]=q[y];q[y]=tmp;}
    ++edits;changed=true;break;
   }
  }
  if(!changed)break;
 }
 return edits;
}
// Normalized stock ADSR Bezier templates sampled at 32 centreline vertices.
// Release is mirrored, exactly as the stock envelope control contract specifies.
inline i32 mix(i32 a,i32 b,i32 t){return a+(b-a)*t/1024;}
inline i32 bez(i32 a,i32 b,i32 c,i32 z,i32 t){return mix(mix(mix(a,b,t),mix(b,c,t),t),mix(mix(b,c,t),mix(c,z,t),t),t);}
// Coordinates remain Q10 through Bezier evaluation and placement.
// Never round a curve to whole pixels before scaling it.
inline i32 extent(i32 v,i32 range){return (v>>16)*range/32767;}
inline void envelope(Frame& f,u32 p,const i32* v,i32 x,i32 y,bool edit,CurveCache* cache=0){
 if(curve_cached(cache,1,v)){
  f.paths[p]=cache->path;
  d::place(f,p,x*1024,y*1024+256,1792,edit?1:0,d::fill_and_stroke);return;
 }
 for(u32 i=0;i<32;++i){
  f.paths[p].points[i]=stock_plot::envelope(v,i);
  f.paths[p].points[63-i]=f.paths[p].points[i];
 }
 curve_ribbon(f,p,true);
 untangle_envelope(f,p);
 u32 hash=2166136261u;for(u32 i=0;i<4;++i)hash=(hash^static_cast<u32>(v[i]>>16))*16777619u;f.paths[p].key=hash|0x80000000u;
 curve_save(cache,1,v,f.paths[p]);
 d::place(f,p,x*1024,y*1024+256,1792,edit?1:0,d::fill_and_stroke);
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
// Match Trine's main page on the five existing Morph frequency-axis marks.
inline i32 trine_x(u32 i){
 const i32 marks[5]={62,139,193,241,298};
 if(i==32)return marks[4];
 return marks[i/8]+(marks[i/8+1]-marks[i/8])*static_cast<i32>(i%8)/8;
}
inline void trine_filter(Frame& f,const i32* v,u32 side,bool edit,CurveCache* cache){
 const u32 edge=2*side+1;
 const i32 row=104*static_cast<i32>(side);
 const i32 geometry[4]={v[0]>>16,v[1]>>16,v[2]>>16,0};
 if(curve_cached(cache,3,geometry))f.paths[edge]=cache->path;
 else{
  i32 pixels[m3_display::curve_points];
  m3_display::sweep_pixels(v[0],v[1],v[2],pixels);
  // The HUD path has room for 32 of Trine's 33 samples. Drop the point
  // closest to its neighbours' line, preserving sharp resonant peaks.
  u32 omit=1;
  i32 smallest=0x7fffffff;
  for(u32 i=1;i<32;++i){
   const i32 left=trine_x(i-1),right=trine_x(i+1);
   const i32 estimate=pixels[i-1]+(pixels[i+1]-pixels[i-1])*(trine_x(i)-left)/(right-left);
   const i32 diff=pixels[i]-estimate;
   const i32 error=diff<0?-diff:diff;
   if(error<smallest){smallest=error;omit=i;}
  }
  for(u32 i=0;i<32;++i){
   const u32 sample=i+(i>=omit?1u:0u);
   const i32 y=25+(pixels[sample]-m3_display::graph_top)*68/96;
   f.paths[edge].points[i]={(trine_x(sample)-190)*512,(y-60)*512};
  }
  curve_ribbon(f,edge);
  u32 key=11;
  for(u32 k=0;k<3;++k)key=(key^static_cast<u32>(geometry[k]))*16777619u;
  f.paths[edge].key=(key&0x7ffffffeu)+1;
  curve_save(cache,3,geometry,f.paths[edge]);
 }
 node(f,edge,190,60+row,2048,edit?1:3,d::fill_and_stroke);
 const u32 center=m3_display::interpolate(m3_display::frequency_table,m3_display::normalized(v[0]),6);
 u32 nearest=0;
 while(nearest<32 && center>(m3_display::plot_frequency[nearest]+m3_display::plot_frequency[nearest+1])/2)++nearest;
 const u32 text=edit?16+side*4:2;
 char q[5]="Q 00";number(q+2,norm(v[1]));
 txt(f,text,d::clamp(trine_x(nearest)-9,62,280),36+row,q,750);
 constexpr u32 times[]={0,150,161,173,186,200,215,231,248,267,287,308,331,356,382,411,442,475,510,548,589,633,680,731,785,844,907,975,1048,1126,1210,1300};
 const u32 ms=times[v[3]>0?static_cast<u32>(v[3])>>26:0];
 char label[8]="0000 ms";
 u32 n=ms;
 for(int k=3;k>=0;--k){label[k]=static_cast<char>('0'+n%10);n/=10;}
 txt(f,text+1,259,88+row,ms?(ms>=1000?label:label+1):"Off",750);
}
// Stock-resource wireframe plots; values stay next to their controls instead
// of occupying prose rows across the graph. All curve coordinates are Q10.
inline void filter(Frame& f,const hud::Snapshot& s,u32 side,CurveCache* cache=0){
 const i32 row=static_cast<i32>(side)*104,cy=60+row;
 const i32* v=(side?s.b:s.a)+14;const bool edit=s.edit_side==side;
 const u32 body=side*2,edge=body+1;
 node(f,ui::hudHorizontal,0,0);
 const char* name=s.filter_type==11?"Trine":s.filter_type==17?"high-pass":s.filter_type==9?"z lowpass":s.filter_type==10?"svf":s.filter_type==16?"ladder":s.filter_type==18?"dynamic":"filter";
 txt(f,side,32,18+row,name,819);symbol(f,side,side?1:0,18,15+row,850);
 if(!(side?s.captured_b:s.captured_a)){txt(f,2+side,62,60+row,"Scene unavailable",819);return;}
 const u32 axis=side?ui::filterAxis1_0:ui::filterAxis0_0;
 for(u32 k=0;k<4;++k)node(f,axis+k,62,94+row);
 const char* labels[5]={"50","1k","2k","5k","20kHz"};const i32 xs[5]={62,139,193,241,298};
 for(u32 k=0;k<5;++k)txt(f,6+side*5+k,xs[k],105+row,labels[k],410);
 if(s.filter_type==11){trine_filter(f,v,side,edit,cache);return;}
 const i32 geometry_values[4]={v[0],s.filter_type==18?v[3]:0,static_cast<i32>(s.filter_type),0};
 if(curve_cached(cache,2,geometry_values))f.paths[edge]=cache->path;
 else {
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
 curve_ribbon(f,edge);
 }
 u32 key=2166136261u;key=(key^static_cast<u32>(v[0]>>16))*16777619u;
 key=(key^static_cast<u32>((s.filter_type==18?v[3]:0)>>16))*16777619u;key=(key^s.filter_type)*16777619u;key&=0x7fffffffu;if(!key)key=1u;
 f.paths[body].key=key;f.paths[edge].key=key;
 if(!curve_cached(cache,2,geometry_values))curve_save(cache,2,geometry_values,f.paths[edge]);
 // The closed body remains hidden: only the response edge is wanted.
 node(f,edge,190,cy,2048,edit?1:3,d::fill_and_stroke);
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
 if(s.module==2){filter(f,s,0,&state.curves[1]);filter(f,s,1,&state.curves[3]);return;}
 node(f,ui::hudHorizontal,0,0);node(f,ui::hudVertical,0,0);
 for(u32 side=0;side<2;++side)for(u32 bank=0;bank<2;++bank){
  const i32 x=10+static_cast<i32>(bank)*177,y=10+static_cast<i32>(side)*104;
  const u32 p=side*2+bank;const bool focus=s.bank_focus==bank;
  if(focus)node(f,ui::envelopeTitle0+p*2,x,y);
  txt(f,focus?24+p:p,x+4,y+14,bank?"FILTER":"AMP",819);
  node(f,ui::envelopeScene0+p*2,x+(bank?51:37)+12,y+10);
  if(side?s.captured_b:s.captured_a)envelope(f,p,(side?s.b:s.a)+(bank?22:8),x+80,y+62,s.edit_side==side && focus,&state.curves[p]);
 }
}
}
