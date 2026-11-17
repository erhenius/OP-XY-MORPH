#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
using namespace morphcapture;
extern "C" i32 sdk_control_display(u32){return 0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot*){return false;}
static unsigned checks=0;
void check(bool b){++checks;if(!b){std::fprintf(stderr,"endpoint check %u failed\n",checks);std::exit(1);}}
int main(){
 const i32 levels[]={0,1,0x10000000,0x40000000,0x60000000,0x7fffffff};
 for(i32 a:levels)for(i32 dec:levels)for(i32 sus:levels)for(i32 rel:levels){
  i32 v[]={a,dec,sus,rel};Frame f={};init_frame(f);envelope(f,0,v,100,60,false);
  check(d::valid(f));check(f.nodes[0].paint==d::fill_and_stroke);
  // Validate thickness in original centreline order. topology.cpp separately
  // verifies the final boundary is uncrossed and preserves these vertices.
  for(u32 i=0;i<32;++i)f.paths[0].points[i]=stock_plot::envelope(v,i);
  curve_ribbon(f,0,true);
  for(u32 i=0;i<32;++i){const auto c=stock_plot::envelope(v,i),l=f.paths[0].points[i],r=f.paths[0].points[63-i];
   check(l.x+r.x==2*c.x && l.y+r.y==2*c.y);
   const i32 dx=l.x-c.x,dy=l.y-c.y,n=dx*dx+dy*dy;
   if(n<317*317 || n>322*322)std::fprintf(stderr,"a=%d d=%d s=%d r=%d i=%u normal=%d,%d n=%d\n",a,dec,sus,rel,i,dx,dy,n);
   check(n>=317*317 && n<=322*322);
  }
  for(u32 i=19;i<=22;++i){
   const auto l=f.paths[0].points[i],rr=f.paths[0].points[63-i];
   check(l.x==rr.x && l.y-rr.y==640);
   check(l.y==f.paths[0].points[19].y && rr.y==f.paths[0].points[44].y);
  }
  // A=0 is vertical, so its flat end face is horizontal; likewise R=max.
  if(!a)check(f.paths[0].points[0].y==f.paths[0].points[63].y);
  if(sus>=0x10000000 && rel==0x7fffffff)check(f.paths[0].points[31].y==f.paths[0].points[32].y);
 }
 // All coincident points must remain finite and deterministic.
 Frame f={};init_frame(f);for(u32 i=0;i<32;++i)f.paths[0].points[i]={0,0};curve_ribbon(f,0);check(d::valid(f));
 std::printf("PASS: %u endpoint geometry checks\n",checks);
}
