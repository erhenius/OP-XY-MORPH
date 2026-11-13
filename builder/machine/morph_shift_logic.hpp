// GUI-only decoded SHIFT snapshot primitives. No packed-word interpolation.
#pragma once
#include <opxy/types.hpp>
namespace morphcapture { namespace shift {
using opxy::u32; using opxy::i32;
struct Schema {
 u32 engine;
 opxy::u8 count, choices[8], defaults[8];
};
inline bool same(const Schema& a,const Schema& b) {
 if(a.engine!=b.engine || a.count!=b.count)return false;
 for(u32 i=0;i<8;++i)
  if(a.choices[i]!=b.choices[i] || a.defaults[i]!=b.defaults[i])return false;
 return true;
}
inline u32 valid(const Schema& s) {
 u32 mask=0;
 for(u32 i=0;i<s.count && i<8;++i)if(s.choices[i]!=1)mask|=1u<<i;
 return mask;
}
// Values are native decoded fields (at most 127); all products fit u32.
inline u32 interpolate(u32 a,u32 b,u32 steps) {
 if(steps>=64)return b;
 return (a*(64-steps)+b*steps+32)/64;
}
struct Snapshot { i32 values[8][8]; u32 valid[8]; };
}}
