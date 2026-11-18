#pragma once
#include <opxy/draw.hpp>
namespace morphcapture { namespace stock_plot {
using opxy::i32;using opxy::u32;
// Positive Q31 control to a fixed-point distance, using bounded 32-bit products.
inline i32 amount(i32 value,u32 distance){
 const u32 q=value<0?0u:static_cast<u32>(value)>>15;
 return static_cast<i32>((distance/65535u)*q+((distance%65535u)*q)/65535u);
}
inline i32 mix(i32 a,i32 b,i32 t){return a+(b-a)*t/1024;}
inline i32 cubic(i32 a,i32 b,i32 c,i32 d,i32 t){return mix(mix(mix(a,b,t),mix(b,c,t),t),mix(mix(b,c,t),mix(c,d,t),t),t);}
// adsr.svg bounds, Q10: a (40.505,203.982)->(156.195,32.544),
// d width 101.097; release starts 333.066 and ends 440.613.
// Stock layout 8d527fdc; cubic construction 8d528848 uses 645/1024
// for attack's second control height and 71/1024 for decay/release.
inline i32 fit(i32 value,i32 span){const i32 n=value*86;return (n/span)*1024+(n%span)*1024/span;}
inline opxy::draw::Point envelope(const i32* v,u32 i){
 const i32 left=41477,right=451188,top=33325,bottom=208878;
 const i32 a=left+amount(v[0],118467),dec=a+amount(v[1],103523);
 const i32 sus=bottom-amount(v[2],175553);
 const i32 release=341060+amount(v[3],110128);
 i32 x,y,t;
 if(i<10){t=static_cast<i32>(i)*1024/9;x=cubic(left,left,left,a,t);y=cubic(bottom,bottom,bottom-(bottom-top)*645/1024,top,t);}
 else if(i<20){t=static_cast<i32>(i-10)*1024/9;x=cubic(a,a,a,dec,t);y=cubic(top,top,sus+(top-sus)*71/1024,sus,t);}
 else if(i<22){t=static_cast<i32>(i-20)*1024;x=mix(dec,release,t);y=sus;}
 else {t=static_cast<i32>(i-22)*1024/9;x=cubic(release,release,release,right,t);y=cubic(sus,sus,bottom+(sus-bottom)*71/1024,bottom,t);}
 // Uniform fit: 400.108 stock pixels -> 86 local pixels, then node scale 1.75.
 return {-44032+fit(x-left,right-left),fit(y-(top+bottom)/2,right-left)};
}
// filter.svg stock screen crop [27,451] x [57.227,172.227]. Uniform
// 0.6 scale gives a 254.4 x 69 pixel plot in each approved HUD cell.
inline i32 filter_x(i32 stock_q10){return 62*1024+(stock_q10-27648)*3/5;}
inline i32 filter_y(i32 stock_q10,i32 row){return (25+row)*1024+(stock_q10-58600)*3/5;}
// Authored cutoff2 response curves from filter.svg (not the polygon closure).
inline opxy::draw::Point filter_response(i32 cutoff,bool hp,u32 i,i32 row){
 const i32 shift=amount(cutoff,384000);i32 x,y;
 if(i==0){x=-524288;y=hp?163840:61440;}
 else if(i==31){x=1048576;y=hp?61440:163840;}
 else {const i32 j=static_cast<i32>(i)-1,t=(j%15)*1024/14;
  if(hp){
   if(j<15){x=cubic(-46080,-20480,2145,12385,t);y=cubic(163840,163840,138240,112640,t);}
   else{x=cubic(12385,22625,40960,66560,t);y=cubic(112640,87040,61440,61440,t);}
  }else{
   if(j<15){x=cubic(30720,56320,74655,84895,t);y=cubic(61440,61440,87040,112640,t);}
   else{x=cubic(84895,95135,117760,143360,t);y=cubic(112640,138240,163840,163840,t);}
  }
  x+=shift;
 }
 return {filter_x(x),filter_y(y,row)};
}
// Stock q_box translate: min(cutoff*375,350), -resonance*65 pixels.
// Authored text anchor (53.7728,156.2457) is retained, then scaled as above.
inline opxy::draw::Point resonance_label(const i32* v,i32 row){
 const i32 x=opxy::draw::clamp(amount(v[0],384000),0,358400);
 const i32 y=-amount(v[1],66560);
 return {filter_x(55063+x),filter_y(159996+y,row)};
}
}}
