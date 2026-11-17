// Tests actual display output at all travel positions and every endpoint combination.
#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace morphcapture;
static i32 words[16]={};static hud::Snapshot scene={};static u32 reads=0,checks=0;
extern "C" i32 sdk_control_display(u32 n){return n<16?words[n]:0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot* out){++reads;*out=scene;return true;}
void require(bool v,const char* message){++checks;if(!v){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
 Frame frame={};DisplayState state={};d::Input in={};in.elapsed=17;
 scene.abi=hud::kSnapshotABI;scene.captured_a=scene.captured_b=scene.start_ready=1;scene.m1.dual=1;
 for(u32 k=0;k<4;++k){scene.m1.primary[k]=k;scene.m1.alternate[k]=k+4;}
 const u32 types[5]={9,10,16,17,18};
 for(u32 type=0;type<5;++type)for(u32 mask=0;mask<16;++mask)for(u32 module=0;module<3;++module)for(u32 shift=0;shift<2;++shift)for(u32 side=0;side<2;++side){
  scene.filter_type=types[type];scene.module=module;scene.shifted=shift;scene.edit_side=side;scene.bank_focus=side;
  for(u32 k=0;k<26;++k){scene.a[k]=(mask&(1u<<(k%4)))?0x7fffffff:0;scene.b[k]=0x7fffffff-scene.a[k];scene.s[k]=scene.current[k]=0x30000000;}
  const hud::Snapshot before=scene;const u32 prior=reads;display(state,in,frame);
  require(d::valid(frame),"Endpoint HUD frame rejected by draw contract");
  if(module==1 && !shift)for(u32 q=0;q<4;++q)require(frame.nodes[q].shade==((q/2==side && q%2==scene.bank_focus)?1u:0u),"M2 highlights an unselected envelope");
  if(module==2 && !shift){for(u32 k=0;k<32;++k)require(!std::strstr(frame.texts[k].chars,"CUT") && !std::strstr(frame.texts[k].chars,"KEY") && !std::strstr(frame.texts[k].chars,"ENV"),"M3 prose labels returned");
   if(scene.filter_type==9 || scene.filter_type==17){const auto& p=frame.paths[1];require(scene.filter_type==17?p.points[0].y>p.points[31].y:p.points[0].y<p.points[31].y,"LP/HP orientation inverted");}}
  require(std::memcmp(&before,&scene,sizeof scene)==0,"Renderer mutated scene data");require(reads==prior+1,"HUD fetched more than one snapshot");
 }
 for(u32 vertical=0;vertical<2;++vertical)for(u32 live=0;live<2;++live){i32 previous=-1000000;
  for(i32 t=-64;t<=64;++t){init_frame(frame);sweep(frame,t,live,vertical);require(d::valid(frame),"Sweep frame invalid");
   const auto& square=frame.nodes[ui::square];const i32 position=vertical?square.y:square.x;
   require(position>=previous,"Sweep square reverses direction");previous=position;
   require((frame.nodes[vertical?ui::capsuleHUD:ui::capsuleAux].x!=640*1024)==(t==0),"Capsule is not midpoint-only");
   if(!t)require(square.x==(vertical?419:240)*1024 && square.y==(vertical?110:147)*1024,"Square not centered");
  }
 }
 in.track=11;for(u32 page=1;page<=3;++page)for(u32 mode=0;mode<2;++mode)for(u32 slot=0;slot<16;++slot){words[4]=page;words[3]=mode<<4;words[8]=slot<<4;display(state,in,frame);require(d::valid(frame),"AUX page invalid");}

 // Sequential retained-paint regressions: cell fills never become selected fills.
 words[4]=3;for(u32 slot=0;slot<16;++slot){words[8]=slot<<4;display(state,in,frame);
  for(u32 k=0;k<16;++k)require(frame.nodes[ui::slot0+k].paint==d::stroke,"Preset cell fill can persist");
  require(frame.nodes[ui::slotSelection].x==(255+static_cast<i32>(slot%4)*54)*1024 && frame.nodes[ui::slotSelection].y==(76+static_cast<i32>(slot/4)*32)*1024,"Preset selection misplaced");}
 in.track=0;scene.module=0;scene.shifted=0;display(state,in,frame);
 for(u32 module=1;module<=2;++module){scene.module=module;scene.shifted=1;display(state,in,frame);
  for(u32 k=0;k<4;++k)require(frame.nodes[ui::row0+k].shade==4 && frame.nodes[ui::row0+k].paint==d::fill_and_stroke,"SHIFT row retained M1 paint");}
 i32 env[4]={0,0x40000000,0x40000000,0x40000000};init_frame(frame);envelope(frame,0,env,90,72,false);
 for(u32 k=0;k<10;++k)require(frame.paths[0].points[k].x==frame.paths[0].points[0].x,"Zero attack is not vertical");
 display_reset(state);words[9]=0;in.track=11;blink(state,in);words[9]=1;in.elapsed=0;blink(state,in);require(icon_shade(state,0)==0,"First flash absent");
 in.elapsed=103;blink(state,in);require(icon_shade(state,0)==6,"First dark interval absent");blink(state,in);require(icon_shade(state,0)==0,"Second flash absent");blink(state,in);require(icon_shade(state,0)==6,"Flash did not end");
 require(norm(0)==0 && norm(0x7fffffff)==99 && norm(-1)==-1,"Display range conversion wrong");
 std::printf("PASS: %u GUI invariants; frame=%zu bytes, display state=%zu bytes\n",checks,sizeof(Frame),sizeof(DisplayState));
}
