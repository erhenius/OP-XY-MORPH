#pragma once
#include <opxy/types.hpp>
namespace morphcapture {
// GUI-owned, elapsed real time; no callback-count or tempo assumption.
struct HudDelay {
 opxy::u32 track, elapsed; bool pending;
 void reset(){track=0xffffffffu;elapsed=0u;pending=false;}
 void observe(opxy::u32 selected,bool eligible,opxy::u32 delta){
  if(!eligible){reset();return;}
  if(!pending || track!=selected){track=selected;elapsed=0u;pending=true;return;}
  const opxy::u32 remaining=150000u-elapsed;
  elapsed+=delta>remaining?remaining:delta;
 }
 bool ready(opxy::u32 selected)const{return pending && track==selected && elapsed>=150000u;}
};
}
