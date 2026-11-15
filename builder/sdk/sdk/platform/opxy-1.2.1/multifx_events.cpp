// MultiFX owns only send-page gestures. Every other raw event reaches the
// authenticated stock entry continuation, including Morph's AUX controls.
#include "multifx_ui.hpp"
bool (*sdk_multifx_raw_event)(const void*) = nullptr;
extern "C" {
void sdk_multifx_event(const void*) __asm__("sdk_multifx_event");
void sdk_multifx_stock_event(const void*) __asm__("sdk_multifx_stock_event");
}
void sdk_multifx_event(const void* event) {
    if (sdk_multifx_raw_event && sdk_multifx_raw_event(event)) return;
    sdk_multifx_stock_event(event);
}
