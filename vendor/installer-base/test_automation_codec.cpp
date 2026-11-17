#include "machine/morph_automation.hpp"
#include <cstdio>
#include <cstdlib>
void check(bool value){if(!value)std::abort();}
int main(){
 using namespace morphcapture::automation;
 for(int t=-64;t<=64;++t){
  int v=encode(t);check(decode(v)==t);
  unsigned stored=static_cast<unsigned>(v)>>16;
  check(decode(static_cast<int>(stored<<16))==t);
 }
 Lane lane={};lane.reset();int t=99;
 check(!lane.observe(encode(64),true,false,t));
 lane.activate();check(lane.observe(encode(-64),true,false,t)&&t==-64);
 check(!lane.observe(encode(-64),true,false,t));
 lane.bypass();check(!lane.observe(encode(64),true,false,t));
 lane.activate();check(!lane.observe(encode(0),true,true,t));
 check(!lane.observe(encode(1),false,false,t));
 std::puts("Movement codec and bypass-priority primitive checks PASS (host only)");
}
