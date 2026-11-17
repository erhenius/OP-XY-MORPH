// Host model: stock pressure already calibrated; no physical sensor emulation.
#include "machine/morph_pad.hpp"
#include <opxy/morph_pad_input.hpp>
#include <cstdio>
#include <cstdlib>
static unsigned checks;
static void require(bool b){++checks;if(!b)std::abort();}
int main(){
 opxy::morphpad::Input input={};
 input.request(false,0);require(!input.event(100));
 input.request(true,100);require(!input.owned);
 require(!input.event(0));require(input.owned);
 require(input.event(-8191));input.request(false,-8191);
 require(input.owned);require(input.event(0));require(!input.owned);
 require(!input.event(8191));
 morphcapture::pad::Rate r={};r.reset();
 r.tick(8191,50000,0,true,false);require(!r.owns); // release to arm
 r.tick(0,50000,0,true,false);
 for(int i=0;i<20;++i)r.tick(8191,50000,0,true,false);
 require(r.target==64 && r.owns);
 r.tick(0,50000,0,true,false);require(r.target==64 && r.owns);
 for(int i=0;i<40;++i)r.tick(-8191,50000,64,true,false);
 require(r.target==-64);
 r.tick(-8191,0,-64,true,false);require(r.target==-64);
 r.tick(8191,50000,-64,true,true);require(!r.owns && !r.armed);
 r.tick(8191,50000,-64,true,false);require(!r.owns);
 r.tick(0,50000,-64,true,false);r.tick(8191,50000,-64,true,false);
 require(r.target==-60);
 r.tick(8191,50000,12,false,false);require(!r.owns && r.target==12);
 // Same duration at varying GUI rates, both directions, light/heavy pressure.
 for(int sign=-1;sign<=1;sign+=2)for(int pressure=1;pressure<=8191;pressure+=819){
  morphcapture::pad::Rate a={},b={};a.reset();b.reset();
  a.tick(0,0,0,true,false);b.tick(0,0,0,true,false);
  for(int i=0;i<100;++i)a.tick(sign*pressure,10000,0,true,false);
  for(int i=0;i<20;++i)b.tick(sign*pressure,50000,0,true,false);
  require(a.target==b.target && a.remainder==b.remainder);
 }
 r.reset();r.tick(0,0,0,true,false);r.tick(2147483647,1000000,0,true,false);
 require(r.target==4); // stalled GUI and malformed pressure bounded
 // Inward sub-step pressure must accumulate at both exact endpoints.
 for(int endpoint=-64;endpoint<=64;endpoint+=128){
  r.reset(endpoint);r.tick(0,0,endpoint,true,false);
  const int inward=endpoint<0?1:-1;
  for(int i=0;i<200;++i)r.tick(inward*100,10000,endpoint,true,false);
  require(inward*(r.target-endpoint)>0);
 }
 // Full pressure travels 96 steps per second (50% above Gate37).
 r.reset(-64);r.tick(0,0,-64,true,false);
 for(int i=0;i<100;++i)r.tick(8191,10000,-64,true,false);
 require(r.target==32);
 for(int gain=-90;gain<=90;gain+=90){
  r.reset(-64);r.tick(0,0,-64,true,false,gain);
  for(int i=0;i<50;++i)r.tick(8191,10000,-64,true,false,gain);
  require(r.target==-64+(48*(100+gain))/100);
 }
 for(unsigned mode=0;mode<3;++mode){
  require(morphcapture::pad::curve(0,mode)==0);
  require(morphcapture::pad::curve(8191,mode)==8191);
  unsigned previous=0;
  for(unsigned x=0;x<=8191;++x){unsigned y=morphcapture::pad::curve(x,mode);require(y>=previous && y<=8191);previous=y;}
 }
 require(morphcapture::pad::curve(2048,0)>2048);
 require(morphcapture::pad::curve(2048,2)<2048);
 for(int sign=-1;sign<=1;sign+=2){r.reset();r.tick(0,0,0,true,false,101);r.tick(sign,1,0,true,false,101);require(r.target==sign*64);r.tick(0,1,0,true,false,101);require(r.target==sign*64);}
 std::printf("PASS: %u pad ownership/rate host checks; Rate=%zu Input=%zu bytes\n",checks,sizeof(r),sizeof(input));
}
