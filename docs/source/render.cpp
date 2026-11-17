#include "../../machine/morphcapture/display.cpp"
#include <cstdio>
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
 Frame f={};DisplayState state={};d::Input in={};in.elapsed=17;
 scene.abi=hud::kSnapshotABI;scene.captured_a=scene.captured_b=scene.start_ready=1;scene.m1.dual=1;scene.mode=1;scene.travel=0;
 for(u32 k=0;k<4;++k){scene.m1.primary[k]=k;scene.m1.alternate[k]=k+4;}
 for(u32 k=0;k<26;++k){scene.a[k]=0x30000000;scene.b[k]=0x50000000;scene.s[k]=scene.current[k]=0x40000000;}
 scene.a[8]=0x10000000;scene.a[14]=0x20000000;scene.a[15]=0x40000000;scene.a[17]=0x60000000;
 scene.b[14]=0x30000000;scene.b[15]=0x20000000;scene.b[17]=0x70000000;
 in.track=11;words[8]=7<<4;words[5]=0x55;words[3]=16;words[2]=0;
 words[4]=1;display(state,in,f);output("aux1_live",f);words[3]=0;display(state,in,f);output("aux1_bypass",f);
 words[4]=2;words[6]=0;display(state,in,f);output("aux2",f);words[6]=1;display(state,in,f);output("aux2_bank",f);
 words[4]=3;words[7]=1|(90<<8);display(state,in,f);output("aux3",f);
 words[3]=16;display(state,in,f);output("aux3_live",f);
 in.track=0;for(u32 page=0;page<3;++page){scene.module=page;scene.filter_type=9;scene.shifted=0;display(state,in,f);char name[12];std::sprintf(name,"hud%u",page+1);output(name,f);}
 scene.filter_type=17;display(state,in,f);output("hud3_hp",f);scene.filter_type=18;display(state,in,f);output("hud3_dynamic",f);
 scene.shifted=1;for(u32 page=1;page<3;++page){scene.module=page;display(state,in,f);char name[14];std::sprintf(name,"hud%u_shift",page+1);output(name,f);}
 in.track=11;words[4]=1;words[3]=16;
 for(int i=0;i<5;++i){words[2]=-64+i*32;display(state,in,f);char name[24];std::sprintf(name,"sweep_%d",i);output(name,f);}

}
