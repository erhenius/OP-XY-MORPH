#include "machine/morph_shift_logic.hpp"
#include "sdk/sdk/platform/opxy-1.2.1/synth_shift.hpp"
#include <cstdio>
#include <cstdlib>
void require(bool b) { if(!b)std::abort(); }
int main() {
 namespace row=opxy::platform::synth_shift;
 namespace logic=morphcapture::shift;
 logic::Schema s={};s.engine=17;s.count=5;
 s.choices[1]=4;s.choices[2]=1;s.defaults[0]=55;s.defaults[1]=2;
 require(logic::valid(s)==27);
 logic::Schema changed=s;require(logic::same(s,changed));
 changed.engine++;require(!logic::same(s,changed));changed=s;
 changed.choices[1]=5;require(!logic::same(s,changed));changed=s;
 changed.defaults[0]++;require(!logic::same(s,changed));
 int word=0;
 require(row::field(word,0,s.defaults)==55);
 word=row::with(word,0,75,s.defaults);
 word=row::with(word,1,3,s.defaults);
 require(row::field(word,0,s.defaults)==75);
 require(row::field(word,1,s.defaults)==3);
 require(row::field(word,2,s.defaults)==0);
 for(unsigned a=0;a<=127;++a)for(unsigned b=0;b<=127;++b) {
  require(logic::interpolate(a,b,0)==a);
  require(logic::interpolate(a,b,64)==b);
  unsigned previous=a;
  for(unsigned t=0;t<=64;++t) {
   unsigned v=logic::interpolate(a,b,t);
   require(v<=127);require(a<=b?v>=previous:v<=previous);previous=v;
  }
 }
 std::puts("SHIFT primitives: SDK packing, defaults, schema identity, exhaustive bounded interpolation PASS");
}
