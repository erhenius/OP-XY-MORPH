#pragma once
#include "morph_hud.hpp"
namespace morphcapture {
using Frame = morph_hud::Frame; // proven V1 HUD: 7 nodes and 18 text leaves
struct DisplayState { opxy::u32 reserved; };
}
