#include <cassert>
#include <cstdio>
#include <opxy/types.hpp>
#include "screen_order.hpp"
using namespace opxy::platform;
int main(){
 ScreenOrder<256> o={};
 ScreenCursor c={0,4};
 auto owned=[](opxy::u32 page){return page==219u?1:0;};
 assert(o.advance(c,88,219,owned));
 assert(o.from==88 && o.to==219);
 c={0,4};assert(o.advance(c,1,219,owned));
 assert(o.to==0); // actual low stock SHIFT page is never substituted
 c={0,4};assert(o.advance(c,88,0,owned));
 assert(o.to==0); // no page requested
 c={0,4};assert(o.advance(c,88,219,owned));
 assert(o.to==219);
 std::puts("PASS Gate10 ScreenOrder honors registered substitute; stock low-page overlays denied");
}
