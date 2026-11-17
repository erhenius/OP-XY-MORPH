#pragma once
#include <opxy/types.hpp>
namespace morphcapture {
// Visual-only timer, advanced by the GUI controller's real elapsed microseconds.
// It neither consumes keys nor authorizes any Morph/stock action.
struct MenuDelay {
 opxy::u32 track,elapsed; bool pending;
 void reset(){track=0xffffffffu;elapsed=0u;pending=false;}
 void observe(opxy::u32 selected,bool eligible,opxy::u32 delta){
  if(!eligible){reset();return;}
  if(!pending || track!=selected){track=selected;elapsed=0;pending=true;return;}
  const opxy::u32 remaining=1500000u-elapsed;
  elapsed+=delta>remaining?remaining:delta;
 }
 bool ready(opxy::u32 selected)const{return pending && track==selected && elapsed>=1500000u;}
};
}
