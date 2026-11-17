#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
using namespace morphcapture;
extern "C" i32 sdk_control_display(u32){return 0;}
extern "C" bool sdk_morph_hud_snapshot_read(u32,hud::Snapshot*){return false;}
static unsigned cases=0,invalid_before=0,maximum=0;
long long orient(d::Point a,d::Point b,d::Point c){return (long long)(b.x-a.x)*(c.y-a.y)-(long long)(b.y-a.y)*(c.x-a.x);}
bool straddle(long long a,long long b){return (a>0 && b<0)||(a<0 && b>0);}
bool crossing(const Frame& f){const auto* q=f.paths[0].points;
 for(int i=0;i<64;++i)for(int j=i+2;j<64;++j){if(i==0 && j==63)continue;
  auto a=q[i],b=q[(i+1)%64],c=q[j],e=q[(j+1)%64];
  if(straddle(orient(a,b,c),orient(a,b,e)) && straddle(orient(c,e,a),orient(c,e,b)))return true;
 }return false;
}
void run(int a,int dec,int sus,int rel){
 i32 v[]={static_cast<i32>(2147483647LL*a/100),static_cast<i32>(2147483647LL*dec/100),static_cast<i32>(2147483647LL*sus/100),static_cast<i32>(2147483647LL*rel/100)};
 Frame f={};init_frame(f);for(u32 i=0;i<32;++i)f.paths[0].points[i]=stock_plot::envelope(v,i);curve_ribbon(f,0,true);
 d::Point before[64];for(int i=0;i<64;++i)before[i]=f.paths[0].points[i];
 for(int i=0;i<64;++i){auto q=f.paths[0].points[i];if(q.x< -45056 || q.x>45056 || q.y< -20480 || q.y>20480)std::exit(4);}
 invalid_before+=crossing(f);u32 edits=untangle_envelope(f,0);maximum=std::max(maximum,edits);++cases;
 if(crossing(f)||!d::valid(f)){std::fprintf(stderr,"FAIL A/D/S/R %d/%d/%d/%d edits %u\n",a,dec,sus,rel,edits);std::exit(1);}
 auto less=[](d::Point x,d::Point y){return x.x<y.x||(x.x==y.x&&x.y<y.y);};
 d::Point after[64];for(int i=0;i<64;++i)after[i]=f.paths[0].points[i];
 std::sort(before,before+64,less);std::sort(after,after+64,less);
 for(int i=0;i<64;++i)if(before[i].x!=after[i].x||before[i].y!=after[i].y)std::exit(2);
 CurveCache cache={};Frame first={},second={};init_frame(first);init_frame(second);envelope(first,0,v,100,60,false,&cache);envelope(second,0,v,100,60,true,&cache);
 for(int i=0;i<64;++i)if(first.paths[0].points[i].x!=second.paths[0].points[i].x||first.paths[0].points[i].y!=second.paths[0].points[i].y)std::exit(3);
}
int main(){
 for(int a=0;a<=100;a+=10)for(int d=0;d<=100;d+=10)for(int s=0;s<=100;s+=10)for(int r=0;r<=100;r+=10)run(a,d,s,r);
 for(int d=30;d<=70;d+=2)for(int s=90;s<=100;++s)for(int r=0;r<=100;r+=10)run(0,d,s,r);
 for(int a=0;a<=100;a+=10)for(int d=0;d<=100;d+=10)for(int s=0;s<=20;++s)run(a,d,s,0);
 std::printf("PASS: %u ADSR cases; %u had crossings before; no crossings after; vertices preserved; max %u repairs; cache identical\n",cases,invalid_before,maximum);
}
