#include "screen_order.hpp"
#include <cstdio>
#include <cstdlib>
static void require(bool pass,const char* message){if(!pass){std::puts(message);std::exit(1);}}
int main(){
 using namespace opxy;using namespace opxy::platform;
 for(u32 selected=2;selected<256;++selected)for(u32 replacement=2;replacement<256;++replacement){
  if(selected==replacement)continue;
  ScreenOrder<256> order={};ScreenCursor cursor={0,0};u32 visits[256]={},position=0,replacement_at=999,menu_at=999;
  do{
   const u32 slot=cursor.slot/4;
   require(slot<256 && visits[slot]++==0,"Duplicate or invalid slot");
   if(slot==replacement)replacement_at=position;
   if(slot==74)menu_at=position;
   ++position;cursor.slot+=4;
  }while(order.advance(cursor,selected,replacement,[=](u32 p){return p==selected?1:0;},[](u32){return 0u;}));
  require(position==255 && visits[selected]==0,"Selected source not suppressed");
  require(replacement_at==1,"Owned replacement not promoted below overlays");
  if(selected!=74 && replacement!=74)require(replacement_at<menu_at,"Held-track menu painted under replacement");
 }
 std::puts("PASS: 64,262 owned-page/replacement combinations; single visits and menu layering");
}
