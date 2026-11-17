#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
using namespace morphcapture;
extern "C" i32 sdk_control_display(u32){return 0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot*){return false;}
static u32 seed=31;
u32 next_word(){seed=seed*1664525u+1013904223u;return seed;}
void check(d::Point a,d::Point b,d::Point c){
 long long exact=(long long)(b.x-a.x)*(c.y-a.y)-(long long)(b.y-a.y)*(c.x-a.x);
 i32 sign=exact<0?-1:exact>0?1:0;
 if(edge_side(a,b,c)!=sign){std::fprintf(stderr,"orientation mismatch\n");std::exit(1);}
}
int main(){
 const d::Point p[]={{-45056,-20480},{45056,-20480},{-45056,20480},{45056,20480},{0,0}};
 for(auto a:p)for(auto b:p)for(auto c:p)check(a,b,c);
 for(unsigned i=0;i<1000000;++i){d::Point q[3];for(auto& v:q)v={static_cast<i32>(next_word()%90113)-45056,static_cast<i32>(next_word()%40961)-20480};check(q[0],q[1],q[2]);}
 std::puts("PASS: 1,000,125 exact orientation comparisons against 64-bit reference");
}
