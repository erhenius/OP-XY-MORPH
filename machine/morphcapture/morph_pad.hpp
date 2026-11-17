// GUI-only rate integration. No writes, recording, allocation or stock calls.
#pragma once
#include <opxy/types.hpp>
namespace morphcapture { namespace pad {
using opxy::i32;using opxy::u32;
// Curves apply to stock-calibrated magnitude, before speed gain.
inline u32 curve(u32 x,u32 mode){
 if(x>8191u)x=8191u;
 if(mode==0u){
  const u32 value=x*8191u;u32 lo=0,hi=8192;
  for(u32 i=0;i<14u;++i){const u32 mid=(lo+hi)/2u;if(mid*mid<=value)lo=mid;else hi=mid;}
  return lo;
 }
 if(mode==2u){
  static const u32 table[17]={0,103,226,372,546,753,998,1291,1638,2051,2543,3127,3822,4649,5632,6801,8191};
  const u32 pos=x*16u,index=pos/8191u,fraction=pos%8191u;
  return index>=16u?8191u:table[index]+(table[index+1]-table[index])*fraction/8191u;
 }
 return x;
}
struct Rate {
 i32 target,direction;u32 remainder;bool armed,owns;
 void reset(i32 current=0){target=current;direction=0;remainder=0;armed=false;owns=false;}
 void tick(i32 pressure,u32 elapsed,i32 current,bool allowed,bool takeover,i32 sensitivity=0,u32 response=1u) {
  if(!allowed || takeover){reset(current);return;}
  if(!pressure){armed=true;direction=0;remainder=0;return;}
  if(!armed)return;
  const i32 sign=pressure<0?-1:1;
  if(!owns){target=current;owns=true;}
  if(sign!=direction){remainder=0;direction=sign;}
  // Native calibration is bounded to +/-8191. Clamp before negation.
  pressure=pressure< -8191?-8191:pressure>8191?8191:pressure;
  if(sensitivity==101){target=sign*64;remainder=0;return;}
  const u32 magnitude=curve(static_cast<u32>(pressure<0?-pressure:pressure),response);
  // Full pressure: approximately 1.33 seconds A->B. Cap stalled GUI time.
  const u32 dt=elapsed>50000u?50000u:elapsed;
  sensitivity=sensitivity< -90?-90:sensitivity>100?100:sensitivity;
  const u32 gain=static_cast<u32>(100+sensitivity);
  const u32 base=magnitude*dt*3u;
  // Split before multiplying: maximum scaled increment + residual < 2^32.
  remainder+=(base/100u)*gain+((base%100u)*gain)/100u;
  const u32 quantum=8191u*31250u;
  const i32 steps=static_cast<i32>(remainder/quantum);
  remainder%=quantum;
  target+=sign*steps;
  if(target>64 || (target==64 && sign>0)){target=64;remainder=0;}
  if(target< -64 || (target==-64 && sign<0)){target=-64;remainder=0;}
 }
};
}}
