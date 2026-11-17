#pragma once
#include "morph_hud.hpp"
#include "ui_ids.hpp"
namespace morphcapture {
using Frame=opxy::draw::Frame<4,64,ui::nodes,32,8>;
struct CurveCache { Frame::path_type path; opxy::i32 values[4]; opxy::u32 kind; };
struct DisplayState { opxy::u32 serial,track; opxy::i32 flash[4]; CurveCache curves[4]; };
}
