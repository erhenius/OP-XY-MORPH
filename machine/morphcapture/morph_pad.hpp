// GUI-only rate integration. No writes, recording, allocation or stock calls.
#pragma once
#include <opxy/types.hpp>
namespace morphcapture { namespace pad {
using opxy::i32;using opxy::u32;
// Curves apply to stock-calibrated magnitude, before speed gain.
inline u32 curve(u32 x,u32 mode){
 if(x>=8191u)return 8191u;
 if(mode==0u){
  // Sample p^0.32 (80% speed at half pressure) at full resolution near zero where the slope is steep.
  static const unsigned short onset[64]={0,458,572,651,714,767,813,854,891,926,957,987,1015,1041,1066,1090,1113,1135,1155,1176,1195,1214,1232,1250,1267,1284,1300,1316,1331,1346,1361,1375,1389,1403,1416,1429,1442,1455,1468,1480,1492,1504,1515,1527,1538,1549,1560,1571,1581,1592,1602,1612,1623,1632,1642,1652,1661,1671,1680,1689,1699,1708,1716,1725};
  static const unsigned short low[8]={1734,2165,2464,2702,2902,3076,3232,3373}; // raw pressure 64..512
  static const unsigned short high[16]={3373,4211,4794,5256,5646,5985,6287,6562,6814,7048,7266,7471,7665,7849,8024,8191}; // raw pressure 512..8192
  if(x<64u)return onset[x];
  if(x<512u){
   const u32 i=(x>>6)-1u;
   return low[i]+((low[i+1]-low[i])*(x&63u)>>6);
  }
  const u32 i=(x>>9)-1u;
  return high[i]+((high[i+1]-high[i])*(x&511u)>>9);
 }
 if(mode==2u){
  // Samples of (32^q - 1)/31, where q = p - 1.2*p*(1-p)*(p-0.5)^2.
  static const u32 table[17]={0,49,118,211,334,492,690,935,1230,1585,2006,2510,3122,3886,4879,6234,8191};
  const u32 pos=x*16u,index=pos/8191u,fraction=pos%8191u;
  return table[index]+(table[index+1]-table[index])*fraction/8191u;
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
  if(sensitivity==201){target=sign*64;remainder=0;return;}
  const u32 magnitude=curve(static_cast<u32>(pressure<0?-pressure:pressure),response);
  // Full pressure: approximately 1.33 seconds A->B. Cap stalled GUI time.
  const u32 dt=elapsed>50000u?50000u:elapsed;
  sensitivity=sensitivity< -90?-90:sensitivity>200?200:sensitivity;
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
