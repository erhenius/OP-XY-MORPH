// Stock-resource contracts: layout, label anchors, clipping and numeric safety.
#include "../machine/morphcapture/display.cpp"
#include <initializer_list>
#include <cstdio>
#include <cstdlib>
using namespace morphcapture;
extern "C" i32 sdk_control_display(u32){return 0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot*){return false;}
static unsigned checks=0;
void check(bool b){++checks;if(!b){std::fprintf(stderr,"geometry check %u failed\n",checks);std::exit(1);}}
int main(){
 const i32 levels[]={0,1,0x10000000,0x40000000,0x60000000,0x7fffffff};
 for(i32 a:levels)for(i32 d:levels)for(i32 s:levels)for(i32 r:levels){
  i32 v[]={a,d,s,r};auto previous=stock_plot::envelope(v,0);
  for(u32 i=0;i<32;++i){auto p=stock_plot::envelope(v,i);check(p.x>=previous.x);check(p.x>=-44032 && p.x<=44032);check(p.y>=-19000 && p.y<=19000);if(!a && i<10)check(p.x==-44032);previous=p;}
  check(previous.x==44032);
 }
 Frame frame={};hud::Snapshot scene={};scene.captured_a=scene.captured_b=1;
 for(u32 type: {9u,17u})for(i32 cut:levels)for(i32 res:levels){
  scene.filter_type=type;scene.a[14]=scene.b[14]=cut;scene.a[15]=scene.b[15]=res;
  init_frame(frame);filter(frame,scene,0);filter(frame,scene,1);
  for(u32 side=0;side<2;++side){const auto& p=frame.paths[side*2+1];
   for(u32 i=0;i<32;++i){check(p.points[i].x>=-65536 && p.points[i].x<=64716);if(i)check(type==17?p.points[i].y<=p.points[i-1].y:p.points[i].y>=p.points[i-1].y);}
  }
  auto p=stock_plot::resonance_label(scene.a+14,0);
  const double cutNorm=double(cut)/2147483647.0,resNorm=double(res)/2147483647.0;
  const double shift=cutNorm*375>350?350:cutNorm*375;
  const double ex=62+(53.7728+shift-27)*.6,ey=25+(156.2457-65*resNorm-57.227)*.6;
  check(std::abs(p.x/1024.0-ex)<.01);check(std::abs(p.y/1024.0-ey)<.01);
 }
 std::printf("PASS: %u stock geometry contracts\n",checks);
}
