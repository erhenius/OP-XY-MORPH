// Read-only retained drawing for the instrument HUD and AUX status.
#include "display.h"
#include <opxy/track_control.hpp>
#include "morph_hud_base_values.hpp"
#include <opxy/morph_hud_snapshot.hpp>
namespace morphcapture {
using opxy::u32;
using opxy::i32;
inline void display_reset(DisplayState& s) { s.reserved=0u; }
inline void aux_face(Frame& frame) {
 morph_hud::Snapshot empty={};
 empty.audible_scene=morph_hud::Unknown;empty.edit_scene=morph_hud::Unknown;
 morph_hud::State neutral={};neutral.enter_m2(11u);
 morph_hud::render(frame,empty,neutral,false,false,false,false,0u);
 opxy::draw::place(frame,7u,0,0,opxy::draw::unit,0u,opxy::draw::keep);
 if(opxy::control::from_control(4u)==3) {
  for(u32 n=0;n<8;++n)opxy::draw::place(frame,53+n,static_cast<i32>(n)*60*opxy::draw::unit,0,opxy::draw::unit,0,opxy::draw::keep);
  for(u32 i=18;i<27;++i)opxy::draw::text(frame,i,640*opxy::draw::unit,320*opxy::draw::unit,opxy::draw::unit,"");
  for(u32 i=0;i<4;++i)opxy::draw::header(frame,i,i==0?"Pad":i==1?"Sensitivity":i==2?"Curve":"Presets");
  opxy::draw::text(frame,27,10*opxy::draw::unit,15*opxy::draw::unit,opxy::draw::unit,"Pad");
  const u32 preset=static_cast<u32>(opxy::control::from_control(8u));
  opxy::draw::text(frame,30,370*opxy::draw::unit,15*opxy::draw::unit,opxy::draw::unit,(preset&1u)?"Back":"Presets");
  if(preset&1u) {
   const char* labels[4]={"Slot","Load A/B","Save A/B","Back"};
   for(u32 i=0;i<4;++i){opxy::draw::header(frame,i,labels[i]);
    opxy::draw::text(frame,27+i,(10+120*static_cast<i32>(i))*opxy::draw::unit,15*opxy::draw::unit,opxy::draw::unit,labels[i]);}
   const u32 slot=((preset>>4u)&15u)+1u;char name[6]="AB 01";
   name[3]=static_cast<char>('0'+slot/10u);name[4]=static_cast<char>('0'+slot%10u);
   opxy::draw::text(frame,22,35*opxy::draw::unit,78*opxy::draw::unit,opxy::draw::unit,name);
   const u32 status=(preset>>8u)&255u;
   const char* message=status==1?"Saved":status==2?"Loaded":status==3?"Loaded: M1 preserved":
       status==4?"Press Save again":status==5?"Stop + BYPASS required":status==6?"Empty or damaged":
       status==7?"File/context error":status==8?"Capture A and B first":"A/B only - S excluded";
   opxy::draw::text(frame,23,35*opxy::draw::unit,127*opxy::draw::unit,opxy::draw::unit,message);
   char tracks[24]="M2/M3 only: --------";const u32 partial=(preset>>16u)&255u;
   for(u32 t=0;t<8;++t)if(partial&(1u<<t))tracks[12+t]=static_cast<char>('1'+t);
   opxy::draw::text(frame,24,35*opxy::draw::unit,179*opxy::draw::unit,opxy::draw::unit,partial?tracks:"K1 slot - K4 back");
   for(u32 n=61;n<63;++n)opxy::draw::place(frame,n,304*opxy::draw::unit,119*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
   return;
  }
  const u32 pad_status=static_cast<u32>(opxy::control::from_control(7u));
  const bool enabled=(pad_status&1u)!=0;
  opxy::draw::text(frame,28,130*opxy::draw::unit,15*opxy::draw::unit,opxy::draw::unit,"Sensitivity");
  const i32 sensitivity=static_cast<i32>((pad_status>>8u)&255u)-90;
  opxy::draw::text(frame,29,250*opxy::draw::unit,15*opxy::draw::unit,opxy::draw::unit,"Curve");
  const u32 response=(pad_status>>16u)&3u;
  opxy::draw::text(frame,26,258*opxy::draw::unit,78*opxy::draw::unit,opxy::draw::unit,response==0?"SQRT":response==2?"EXP":"LINEAR");
  const u32 magnitude=static_cast<u32>(sensitivity<0?-sensitivity:sensitivity);
  char percent[8]={};u32 at=0;
  if(sensitivity)percent[at++]=sensitivity<0?'-':'+';
  if(magnitude>=100)percent[at++]='1';
  if(magnitude>=10)percent[at++]=static_cast<char>('0'+(magnitude/10)%10);
  percent[at++]=static_cast<char>('0'+magnitude%10);percent[at++]='%';
  opxy::draw::text(frame,25,130*opxy::draw::unit,78*opxy::draw::unit,opxy::draw::unit,sensitivity==101?"INSTANT":percent);
  opxy::draw::text(frame,22,35*opxy::draw::unit,78*opxy::draw::unit,opxy::draw::unit,enabled?"MORPH":"PITCH");
  opxy::draw::text(frame,23,35*opxy::draw::unit,127*opxy::draw::unit,opxy::draw::unit,enabled?"Pressure = speed":"Stock pitch bend");
  opxy::draw::text(frame,24,35*opxy::draw::unit,179*opxy::draw::unit,opxy::draw::unit,(pad_status&2u)?"Release to switch":enabled?"Release holds position":"K1 selects pad mode");
  for(u32 n=61;n<63;++n)opxy::draw::place(frame,n,304*opxy::draw::unit,119*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
  return;
 }
 if(opxy::control::from_control(4u)==2) {
  const u32 bank=static_cast<u32>(opxy::control::from_control(6u))&1u;
  const u32 mask=static_cast<u32>(opxy::control::from_control(5u));
  const u32 sizes[8]={12,12,12,12,14,14,20,26};
  for(u32 t=0;t<8;++t){
   const i32 x=(148+32*static_cast<i32>(t))*opxy::draw::unit;
   opxy::draw::place(frame,8+t,x,75*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
   const bool selected=t/4u==bank;
   if(selected)opxy::draw::place(frame,16+t%4u,x,75*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
   const u32 digit=20+t+((selected && t%4u<2u)?0u:8u);
   opxy::draw::place(frame,digit,x,75*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
   opxy::draw::place(frame,(mask&(1u<<t)?36u:44u)+t,x,112*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
   opxy::draw::text(frame,18+t,(162+32*static_cast<i32>(t)-8)*opxy::draw::unit,
       70*opxy::draw::unit,10*opxy::draw::unit/static_cast<i32>(sizes[t]),(mask&(1u<<t))?"on":"off");
  }
  // M2 statuses are uniformly light, irrespective of their M1 header shade.
  opxy::draw::place(frame,52,0,0,opxy::draw::unit,0,opxy::draw::keep);
  for(u32 n=61;n<63;++n)opxy::draw::place(frame,n,35*opxy::draw::unit,837*opxy::draw::unit/10,623,0,opxy::draw::keep);
  opxy::draw::text(frame,26,35*opxy::draw::unit,70*opxy::draw::unit,opxy::draw::unit,"in");
  return;
 }
 for(u32 n=0;n<8;++n)opxy::draw::place(frame,53+n,static_cast<i32>(n)*60*opxy::draw::unit,0,opxy::draw::unit,0,opxy::draw::keep);
 for(u32 n=61;n<63;++n)opxy::draw::place(frame,n,304*opxy::draw::unit,119*opxy::draw::unit,opxy::draw::unit,0,opxy::draw::keep);
 for(u32 i=18;i<22;++i)opxy::draw::text(frame,i,640*opxy::draw::unit,320*opxy::draw::unit,opxy::draw::unit,"");
 const char* labels[4]={"A","B","S","Morph"};
 for(u32 i=0u;i<4u;++i) {
  opxy::draw::header(frame,i,labels[i]);
  opxy::draw::text(frame,27u+i,(10+120*static_cast<i32>(i))*opxy::draw::unit,
      15*opxy::draw::unit,opxy::draw::unit,labels[i]);
 }
 hud::Snapshot status={};
 const bool valid=sdk_morph_hud_snapshot_read(11u,&status) && status.abi==hud::kSnapshotABI;
 u32 ca=0u,cb=0u;
 for(u32 t=0;t<8u;++t){ca+=(status.captured_a>>t)&1u;cb+=(status.captured_b>>t)&1u;}
 char captures[24]="A captured: 0/8";
 captures[12]=valid?static_cast<char>('0'+ca):'-';
 opxy::draw::text(frame,22u,35*opxy::draw::unit,78*opxy::draw::unit,opxy::draw::unit,captures);
 captures[0]='B';captures[12]=valid?static_cast<char>('0'+cb):'-';
 opxy::draw::text(frame,23u,258*opxy::draw::unit,78*opxy::draw::unit,opxy::draw::unit,captures);
 opxy::draw::text(frame,24u,35*opxy::draw::unit,127*opxy::draw::unit,opxy::draw::unit,
     valid?(status.mode?"LIVE":"BYPASS"):"--");
 char amount[24]="0%";
 if(valid && status.mode && status.travel) {
  const i32 bounded=status.travel < -64 ? -64 : status.travel > 64 ? 64 : status.travel;
  const u32 percent=static_cast<u32>(bounded<0?-bounded:bounded)*100u/64u;
  u32 at=0u;if(percent>=100u)amount[at++]='1';
  if(percent>=10u)amount[at++]=static_cast<char>('0'+(percent/10u)%10u);
  amount[at++]=static_cast<char>('0'+percent%10u);amount[at++]='%';amount[at++]=' ';
  amount[at++]='t';amount[at++]='o';amount[at++]=' ';amount[at++]=bounded<0?'A':'B';amount[at]=0;
 }
 if(!valid){amount[0]='-';amount[1]='-';amount[2]=0;}
 opxy::draw::text(frame,25u,35*opxy::draw::unit,179*opxy::draw::unit,opxy::draw::unit,amount);
}
inline void display(DisplayState&,const opxy::draw::Input& input,Frame& frame) {
 for(u32 n=8;n<63;++n)opxy::draw::hide(frame,n);
 for(u32 i=27;i<31;++i)opxy::draw::text(frame,i,640*opxy::draw::unit,320*opxy::draw::unit,opxy::draw::unit,"");
 opxy::draw::text(frame,26,640*opxy::draw::unit,320*opxy::draw::unit,opxy::draw::unit,"");
 if(input.track==11u){aux_face(frame);return;}
 morph_hud::Snapshot view={};
 view.audible_scene=morph_hud::Unknown;
 view.edit_scene=morph_hud::Unknown;
 morph_hud::State state={};state.enter_m2(input.track);
 hud::Snapshot snap={};
 bool ready=input.track<8u && sdk_morph_hud_snapshot_read(input.track,&snap) &&
            snap.abi==hud::kSnapshotABI && snap.track==input.track;
 // All displayed values are drawn from one verified real-scene snapshot.
 const u32 saved_slot[26]={0u,1u,2u,3u,4u,5u,6u,7u,8u,9u,10u,11u,
                            13u,15u,16u,17u,18u,19u,20u,21u,22u,23u,32u,33u,34u,35u};
 const bool m1=ready && snap.module==0u,m3=ready && snap.module==2u;
 const bool shifted=ready && snap.shifted!=0u;
 const bool dual=m1 && snap.m1.dual!=0u;
 const bool secondary=m1 && snap.m1.secondary!=0u;
 if(ready) {
  view.edit_scene=snap.edit_side?morph_hud::B:morph_hud::A;
  state.bank=snap.bank_focus?morph_hud::Filter:morph_hud::Amp;
  for(u32 bank=0u;bank<2u;++bank)for(u32 col=0u;col<4u;++col) {
   const u32 physical=shifted && !m1 ? (bank ? 0xffffffffu :
      (m3 ? 20u+col : col==1u?13u:col==3u?15u:0xffffffffu)) : m1 ? (bank ? snap.m1.alternate[col] : snap.m1.primary[col]) :
      m3 ? (bank ? 0xffffffffu : 16u+col) : (bank ? 32u+col : 8u+col);
   u32 at=26u;
   for(u32 i=0;i<26u;++i)if(saved_slot[i]==physical){at=i;break;}
   if(at==26u)continue; // Unknown or absent M1 slot displays --, never fake data.
   if(snap.start_ready)view.cells[bank][morph_hud::Base][col]=morph_hud_base::normalized(snap.s[at]);
   if(snap.captured_a)view.cells[bank][morph_hud::RowA][col]=morph_hud_base::normalized(snap.a[at]);
   if(snap.captured_b)view.cells[bank][morph_hud::RowB][col]=morph_hud_base::normalized(snap.b[at]);
  }
 }
 // V1 sweeper is current travel, NOT an inferred S midpoint.
 const i32 travel=ready ? snap.travel : 0;
 const u32 magnitude=static_cast<u32>(travel < 0 ? -travel : travel);
 const u32 bounded=magnitude>64u?64u:magnitude;
 const u32 meter= (ready && snap.mode ? 4u : 8u) |
            ((bounded*100u/64u) << 8u) |
            (travel<0 ? 16u : travel>0 ? 32u : 0u) |
            (ready && snap.fault ? 0x00040000u | ((snap.fault&255u)<<20u) : 0u);
 morph_hud::render(frame,view,state,m3,m1,dual,secondary,meter,shifted,64u,4u,shifted && !m1);
 // Keep the CV-only cover and diagnostics off the normal instrument HUD.
 opxy::draw::hide(frame,7u);
 for(u32 i=18u;i<26u;++i)
  opxy::draw::text(frame,i,opxy::draw::hidden_x*opxy::draw::unit,
                   opxy::draw::hidden_y*opxy::draw::unit,opxy::draw::unit,"");

}
}
