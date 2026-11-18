#pragma once
#include "morph_hud.hpp"
#include <opxy/track_control.hpp>
namespace morphcapture { namespace legacy_ui {
using Frame=morph_hud::Frame;using opxy::u32;using opxy::i32;
inline void aux_face(Frame& frame){
 for(u32 n=0;n<63;++n)opxy::draw::hide(frame,n);
 for(u32 n=0;n<31;++n)opxy::draw::text(frame,n,640*1024,320*1024,1024,"");
 opxy::draw::place(frame,7,0,0,1024,0,opxy::draw::keep);
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
}
}}
