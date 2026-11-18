#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace morphcapture;
extern "C" i32 sdk_control_display(u32){return 0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot*){return false;}
static unsigned checks=0;
void check(bool b){++checks;if(!b){std::fprintf(stderr,"FAIL dynamic contract %u\n",checks);std::exit(1);}}
int main(){
 for(u32 side=0;side<2;++side)for(u32 edit=0;edit<2;++edit){
  hud::Snapshot s={};s.filter_type=18;s.captured_a=s.captured_b=1;s.edit_side=edit;
  i32* v=(side?s.b:s.a)+14;v[0]=0x20000000;v[1]=0x10000000;v[2]=0x30000000;v[3]=0x60000000;
  Frame base={};init_frame(base);filter(base,s,side);
  u32 text=edit==side?16+side*4:2;
  check(!std::strcmp(base.texts[text+1].chars,"C 12"));
  check(!std::strcmp(base.texts[text+2].chars,"D 37"));
  for(u32 k=0;k<4;++k){Frame f={};i32 old=v[k];v[k]+=0x10000000;init_frame(f);filter(f,s,side);v[k]=old;
   for(u32 meter=0;meter<2;++meter)check((f.nodes[ui::dynamicBar0+side*2+meter].scale!=base.nodes[ui::dynamicBar0+side*2+meter].scale)==(k==meter+1));
   const auto& a=base.paths[side*2+1];const auto& b=f.paths[side*2+1];
   check((std::memcmp(a.points,b.points,sizeof a.points)!=0)==(k==0 || k==3));
   if(k==0)for(u32 i=16;i<32;++i)check(a.points[i].y==b.points[i].y);
   if(k==3)for(u32 i=0;i<16;++i)check(a.points[i].y==b.points[i].y);
   if(k==1 || k==2)check(a.key==b.key);
   for(u32 n=0;n<4;++n)check((std::strcmp(f.texts[text+n].chars,base.texts[text+n].chars)!=0)==(n==k));
  }
 }
 std::printf("PASS: %u dynamic control-isolation checks\n",checks);
}
