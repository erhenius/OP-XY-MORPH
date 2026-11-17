#include <cstdio>
#include "display.cpp"
static int page=1,bank=0,pad=0,preset=0;
extern "C" opxy::i32 sdk_control_display(opxy::u32 i){return i==4?page:i==5?0xfc:i==6?bank:i==7?(pad|(191<<8)|(2<<16)):i==8?preset:0;}
extern "C" bool sdk_morph_hud_snapshot_read(opxy::u32,morphcapture::hud::Snapshot* out){*out={};out->abi=morphcapture::hud::kSnapshotABI;out->captured_a=255;out->captured_b=255;out->mode=1;out->travel=32;return true;}
int main(){
 for(int scene=0;scene<8;++scene){page=scene>=3?3:scene?2:1;bank=scene==2?1:0;pad=scene==4?1:0;preset=scene>=5?1|(15<<4)|((scene==5?0:scene==6?4:3)<<8)|((scene==7?129:0)<<16):0;
  morphcapture::Frame f={};morphcapture::DisplayState s={};opxy::draw::Input in={};in.track=11;
  morphcapture::display(s,in,f);
  std::printf("FRAME %d\n",scene);
  for(unsigned i=0;i<f.node_count;++i){auto n=f.nodes[i];std::printf("N %u %d %d %d %d %d\n",i,n.x,n.y,n.scale,n.cos,n.sin);}
  for(unsigned i=0;i<f.text_count;++i){auto t=f.texts[i];std::printf("T %u %d %d %d %s\n",i,t.x,t.y,t.scale,t.chars);}
 }
}
