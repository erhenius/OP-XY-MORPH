// Portable A/B pairs. No S, movement origin, route mask or playback state.
#pragma once
#include <opxy/morph_store_bridge.hpp>
namespace morph_presets {
using opxy::u32; using opxy::i32; using opxy::u8;
using opxy::morphport::ShiftSchema;
const u32 bytes=2396u, values_at=216u, slots=16u;
inline u32 get(const u8* p){u32 v=0;for(u32 n=0;n<4;++n)v|=u32(p[n])<<(8*n);return v;}
inline void put(u8* p,u32 v){for(u32 n=0;n<4;++n)p[n]=u8(v>>(8*n));}
inline u32 checksum(const u8* p){u32 h=2166136261u;for(u32 n=0;n<bytes-4;++n)h=(h^p[n])*16777619u;return h;}
inline bool same(const ShiftSchema& a,const ShiftSchema& b){
 if(a.engine!=b.engine || a.count!=b.count)return false;
 for(u32 n=0;n<8;++n)if(a.choices[n]!=b.choices[n] || a.defaults[n]!=b.defaults[n])return false;
 return true;
}
inline ShiftSchema schema(const u8* p,u32 track){
 const u8* q=p+24+24*track;ShiftSchema s={};s.engine=get(q);s.count=u8(get(q+4));
 for(u32 n=0;n<8;++n){s.choices[n]=q[8+n];s.defaults[n]=q[16+n];}return s;
}
inline bool valid(const u8* p,u32 size){
 if(size!=bytes || get(p)!=0x3142414du || get(p+4)!=1u || get(p+8)!=bytes ||
    get(p+16)>255u || get(p+20)>255u || get(p+bytes-4)!=checksum(p))return false;
 for(u32 t=0;t<8;++t){
  if(get(p+24+24*t+4)>8u)return false;
  const ShiftSchema s=schema(p,t);
  for(u32 n=0;n<8;++n){const u32 c=s.choices[n],top=c>=2?c-1:c==1?0:100;
   if(c>128u || s.defaults[n]>top)return false;
  }
  for(u32 side=0;side<2;++side)for(u32 n=0;n<34;++n){
   const u32 v=get(p+values_at+4*((side*8+t)*34+n));
   if(v>0x7fffffffu)return false;
   if(n>=26u){const u32 e=n-26,c=s.choices[e];
    const u32 top=e>=s.count || c==1?0:c>=2?c-1:100;
    if(v>top)return false;
   }
  }
 }
 return true;
}
inline void encode(u8* p,u32 generation,const i32 (&scene)[2][8][34],
                   const u32 (&masks)[2],const ShiftSchema* schemas){
 put(p,0x3142414du);put(p+4,1);put(p+8,bytes);put(p+12,generation);
 put(p+16,masks[0]);put(p+20,masks[1]);
 for(u32 t=0;t<8;++t){u8* q=p+24+24*t;put(q,schemas[t].engine);put(q+4,schemas[t].count);
  for(u32 n=0;n<8;++n){q[8+n]=schemas[t].choices[n];q[16+n]=schemas[t].defaults[n];}}
 for(u32 b=0;b<2;++b)for(u32 t=0;t<8;++t)for(u32 n=0;n<34;++n)
  put(p+values_at+4*((b*8+t)*34+n),u32(scene[b][t][n]));
 put(p+bytes-4,checksum(p));
}
// The shared stock M2/M3 fields are indices 8..25 in Morph's scene layout.
// M1 (0..7) and synth SHIFT (26..33) require an exact engine/schema match.
// Validate before calling; absent captures preserve that whole destination.
inline u32 merge(const u8* p,i32 (&scene)[2][8][34],u32 (&masks)[2],const ShiftSchema* current){
 u32 partial=0;
 for(u32 t=0;t<8;++t){const bool full=same(schema(p,t),current[t]);
  for(u32 b=0;b<2;++b){if(!(get(p+16+4*b)&(1u<<t)))continue;
   // Partial recall needs an existing complete scene to preserve its M1.
   if(!full && !(masks[b]&(1u<<t)))continue;
   for(u32 n=0;n<34;++n)if(full || (n>=8 && n<26))
    scene[b][t][n]=i32(get(p+values_at+4*((b*8+t)*34+n)));
   masks[b]|=1u<<t;if(!full)partial|=1u<<t;
  }
 }return partial;
}
inline bool newer(u32 a,u32 b){return a!=b && (a-b)<0x80000000u;}
}
