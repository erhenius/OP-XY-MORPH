// MIDI movement mapping and manual source ownership; no recording path.
#pragma once
#include <opxy/types.hpp>
namespace morphcapture { namespace midi {
using opxy::u32;using opxy::i32;
inline i32 travel(u32 cc){
 if(cc>127u)cc=127u;
 return cc<=63u?static_cast<i32>((cc*64u+31u)/63u)-64:
     static_cast<i32>(((cc-64u)*64u+31u)/63u);
}
struct State {
 i32 target;bool ready,owns;
 void reset(){target=0;ready=false;owns=false;}
 void offer(u32 cc){target=travel(cc);ready=true;owns=true;}
};
}}
