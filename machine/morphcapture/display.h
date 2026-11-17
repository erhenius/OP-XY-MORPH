#pragma once
#include "morph_hud.hpp"
#include "ui_ids.hpp"
namespace morphcapture {
using Frame=opxy::draw::Frame<4,64,ui::nodes,32,8>;
struct DisplayState { opxy::u32 serial,track; opxy::i32 flash[4]; };
}
