// Pure movement codec; native stock automation keeps a 16-bit parameter word.
#pragma once
#include <opxy/types.hpp>
namespace morphcapture { namespace automation {
using opxy::u32;using opxy::i32;
// 129 positions with a stable 16-bit representation. Zero travel is Q31 0x40000000 (stored high half 0x4000).
inline i32 encode(i32 travel) {
 travel=travel< -64?-64:travel>64?64:travel;
 return travel==64?0x7fffffff:(travel+64)*0x01000000;
}
inline i32 decode(i32 value) {
 if(value<0)value=0;
 const u32 rounded=(static_cast<u32>(value)+0x00800000u)>>24u;
 return static_cast<i32>(rounded>128u?128u:rounded)-64;
}
struct Lane {
 i32 seen;
 bool primed,blocked,was_playing;
 void reset(){seen=0;primed=false;blocked=true;was_playing=false;}
 void bypass(){blocked=true;}
 void activate(){blocked=false;}
 bool observe(i32 value,bool playing,bool muted,i32& travel) {
  const bool changed=!primed || value!=seen || (playing && !was_playing);
  was_playing=playing;
  seen=value;primed=true;
  if(!playing || muted || blocked || !changed)return false;
  travel=decode(value);return true;
 }
};
}}
