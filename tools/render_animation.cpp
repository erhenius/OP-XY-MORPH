#include "../machine/morphcapture/display.cpp"
#include <cstdio>
#include <cmath>
using namespace morphcapture;
static i32 words[16]={};static hud::Snapshot scene={};
extern "C" i32 sdk_control_display(u32 n){return n<16?words[n]:0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot* out){*out=scene;return true;}
void output(const char* name,Frame& f){
 std::printf("{\"name\":\"%s\",\"nodes\":[",name);
 for(u32 i=0;i<Frame::node_count;++i){const auto& n=f.nodes[i];std::printf("%s[%d,%d,%d,%d,%d,%u,%u]",i?",":"",n.x,n.y,n.scale,n.cos,n.sin,n.shade,n.paint);}
 std::printf("],\"paths\":[");for(u32 p=0;p<4;++p){std::printf("%s[",p?",":"");for(u32 i=0;i<64;++i)std::printf("%s[%d,%d]",i?",":"",f.paths[p].points[i].x,f.paths[p].points[i].y);std::printf("]");}
 std::printf("],\"texts\":[");for(u32 i=0;i<32;++i){const auto& t=f.texts[i];std::printf("%s[%d,%d,%d,\"%s\"]",i?",":"",t.x,t.y,t.scale,t.chars);}std::puts("]}");
}
int main(){
 Frame f={};DisplayState state={};d::Input in={};in.elapsed=83;
 for(int frame=0;frame<144;++frame){
  const double phase=frame*6.283185307179586/144;
  const double wave=(1-std::cos(phase))*.5;
  scene={};scene.abi=hud::kSnapshotABI;scene.captured_a=scene.captured_b=scene.start_ready=1;scene.m1.dual=1;
  scene.mode=1;scene.travel=static_cast<i32>(64*std::sin(phase));
  for(u32 k=0;k<4;++k){scene.m1.primary[k]=k;scene.m1.alternate[k]=k+4;}
  for(u32 k=0;k<26;++k){scene.a[k]=0x30000000;scene.b[k]=0x50000000;scene.s[k]=scene.current[k]=0x40000000;}
  scene.a[8]=0x10000000;scene.a[14]=0x20000000;scene.a[15]=0x40000000;scene.a[17]=0x60000000;
  scene.b[14]=0x30000000;scene.b[15]=0x20000000;scene.b[17]=0x70000000;
  for(int tile=0;tile<8;++tile){
   char name[32];std::sprintf(name,"%03d_%d",frame,tile);
   if(tile<3){
    in.track=11;words[4]=tile+1;words[3]=tile==0?16:0;words[2]=scene.travel;
    words[5]=0x55^(1u<<((frame/18)%8));words[6]=(frame/72)%2;
    words[8]=((frame/9)%16)<<4;words[7]=1|((90+static_cast<int>(65*std::sin(phase)))<<8)|((frame/48)<<17)|((39+static_cast<int>(37*std::sin(phase+.8)))<<20);
   }else{
    in.track=0;scene.shifted=tile==7;scene.module=tile==3?0:tile==4?1:2;scene.filter_type=tile==6?18:9;
    scene.mode=tile==3?1:0;
    if(tile>=4)scene.travel=0;
    if(tile==4){scene.a[8]=static_cast<i32>((.08+.65*wave)*2147483647.0);scene.a[10]=static_cast<i32>((.2+.6*wave)*2147483647.0);}
    if(tile>=5){scene.a[14]=static_cast<i32>((.12+.45*wave)*2147483647.0);scene.a[15]=static_cast<i32>((.15+.65*wave)*2147483647.0);scene.a[16]=static_cast<i32>((.65-.45*wave)*2147483647.0);scene.a[17]=static_cast<i32>((.65+.28*wave)*2147483647.0);}
    if(tile==7)for(int j=18;j<22;++j)scene.a[j]=static_cast<i32>((.15+.65*(1-std::cos(phase+(j-18)*.7))*.5)*2147483647.0);
   }
   display(state,in,f);output(name,f);
  }
 }
}
