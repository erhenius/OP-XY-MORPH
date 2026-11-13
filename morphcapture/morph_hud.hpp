// XY Morph v1.2.9c: read-only M2 HUD state/renderer, within SDK text budget.
// No stock parameter writes, no device input interception, no HUD screen hook.
#pragma once
#include <opxy/draw.hpp>

namespace morph_hud {
using opxy::i32;
using opxy::u32;
namespace d = opxy::draw;

enum Bank : u32 { Amp = 0, Filter = 1 };
enum Scene : u32 { A = 0, B = 1, Unknown = 2 };
enum Row : u32 { RowA = 0, Base = 1, RowB = 2 };
enum Action : u32 { None = 0, FocusChanged = 1, RequestGlobalEditToggle = 2 };

struct Cell { i32 display_integer; bool verified; };
struct Snapshot {
    Cell cells[2][3][4];
    Scene audible_scene; // Live/morph/bypass truth; NEVER the edit destination.
    Scene edit_scene;    // RED row; comes from the real Morph controller.
};
struct State {
    Bank bank;
    u32 track;
    bool open;
    void enter_m2(u32 index) { track = index; bank = Amp; open = true; }
    void leave_m2() { open = false; }
    Action encoder_push(u32 encoder) {
        if (!open) return None;
        if (encoder == 0) { bank = bank == Amp ? Filter : Amp; return FocusChanged; }
        if (encoder == 1) return RequestGlobalEditToggle;
        return None;
    }
};

// SDK resource ceilings: texts <= 32; v1.2.9b mistakenly used 53.
// v1.2.9c uses one text leaf per entire parameter row, with a red duplicate
// for each A/B row. Fixed four-character columns keep labels and ADSR values
// grouped. Actual device font alignment needs a hardware visual check.
// 0..5 ordinary rows, 6..9 red duplicates, 10..11 bank labels,
// 12..13 inverted bank labels, 14 large audible A/B.
using Frame = d::Frame<0, 0, 63, 31, 1>;
constexpr u32 plain_row(u32 bank, u32 row) { return bank*3u + row; }
constexpr u32 red_row(u32 bank, u32 row) { return 6u + bank*2u + (row == RowB ? 1u : 0u); }
constexpr u32 ordinary_bank(u32 bank) { return 10u + bank; }
constexpr u32 inverted_bank(u32 bank) { return 12u + bank; }
constexpr u32 audible_letter = 14u;
// Registered text leaves ensure A and B render like every other HUD label.
constexpr u32 meter_letter_a = 15u;
constexpr u32 meter_letter_b = 16u;
constexpr u32 shift_diagnostic = 17u;

inline void append_padded(char (&target)[24], u32& at, const char* text, u32 width) {
    u32 size=0;
    while (text[size] && size < width) ++size;
    while (size < width && at < 23u) { target[at++]=' '; ++size; }
    for (u32 i=0;text[i] && at<23u;++i) target[at++]=text[i];
    target[at]=0;
}
inline void append_label(char (&target)[24], u32& at, const char* label) {
    u32 i=0;
    while (label[i] && at<23u) target[at++]=label[i++];
    while (i++<4u && at<23u) target[at++]=' ';
    target[at]=0;
}
inline void format_cell(char (&buffer)[8], Cell cell) {
    if (!cell.verified || cell.display_integer < 0 || cell.display_integer > 999) {
        d::copy(buffer, sizeof buffer, "--"); return;
    }
    char reverse[3];
    u32 n=static_cast<u32>(cell.display_integer), len=0;
    do { reverse[len++]=static_cast<char>('0'+n%10u); n/=10u; } while (n && len<3u);
    u32 at=0;
    while (len) buffer[at++]=reverse[--len];
    buffer[at]=0;
}
inline void row_string(char (&out)[24], const Snapshot& snap,u32 bank,u32 row) {
    const char* label=row==RowA ? "A" : row==RowB ? "B" : "S";
    u32 at=0;
    append_label(out,at,label);
    for (u32 k=0;k<4u;++k) {
        char number[8]; format_cell(number,snap.cells[bank][row][k]);
        append_padded(out,at,number,4);
    }
}
inline void place_text(Frame& frame,u32 id,i32 x,i32 y,const char* value) {
    d::text(frame,id,x*d::unit,y*d::unit,d::unit,value);
}
inline void hide_text(Frame& frame,u32 id,const char* value) {
    d::text(frame,id,d::hidden_x*d::unit,d::hidden_y*d::unit,d::unit,value);
}
// The controller's existing packed display word is a read-only position feed.
// 0..100% from its live origin toward scene A (negative) or B (positive).
// Bypass is NOT inferred from the red editing row.
inline void draw_meter(Frame& frame,u32 packed) {
    const bool active=(packed&4u)!=0u && (packed&8u)==0u;
    const u32 percent=(packed>>8u)&127u;
    const u32 bounded=percent>100u ? 100u : percent;
    const bool a=(packed&16u)!=0u, b=(packed&32u)!=0u;
    const i32 delta=static_cast<i32>(bounded*79u/100u);
    const i32 y=100+(a && !b ? -delta : (b && !a ? delta : 0));
    // Zero is the only sparse white LIVE point. Intermediate travel gets
    // a thin solid line; exactly 100% gets a visibly heavier solid bar.
    // Bypass is always sparse red regardless of the retained position.
    const u32 marker=!active ? 4u : (bounded==0u ? 3u :
                      (bounded==100u && (a!=b) ? 2u : 5u));
    for(u32 n=2u;n<6u;++n) {
        d::place(frame,n,0,y*d::unit,d::unit,0,d::keep);
        if(n!=marker) d::hide(frame,n);
    }
}
inline void render(Frame& frame,const Snapshot& source,const State& state,bool m3=false,bool m1=false,
                   bool m1_dual=false,bool m1_secondary=false,u32 morph_meter=0u,bool shift_chord=false,
                   u32 shift_stock_setter_index=64u,u32 shift_raw_knob=4u,
                   bool shift_numeric=false) {
    const bool numeric=m3||m1||shift_numeric;
    const u32 bank_focus=m1 && m1_dual ? (m1_secondary ? 1u : 0u) : (numeric ? 0u : (state.bank == Filter ? 1u : 0u));
    for (u32 bank=0;bank<2u;++bank) {
        d::place(frame,bank,0,0,d::unit,0,d::keep);
        if ((numeric && !(m1 && m1_dual)) || bank != bank_focus) d::hide(frame,bank);
    }
    // Only M1 synths with a real second row show the lower section.
    // The renderer's extra registered mask covers stock-styled lower rows
    // for single-bank M1; it is hidden for M2/M3 and dual-bank M1.
    if (m1 && !m1_dual) d::place(frame,6u,0,0,d::unit,0,d::keep);
    else d::hide(frame,6u);
    const i32 bank_y[2]={33,131};
    const i32 row_y[2][3]={{33,58,83},{131,156,181}};
    const char* bank_title[2]={"AMP","FILTER"};
    for (u32 bank=0;bank<2u;++bank) {
        if (numeric && !(m1 && m1_dual) && bank==1u) {
            hide_text(frame,ordinary_bank(bank),bank_title[bank]);
            hide_text(frame,inverted_bank(bank),bank_title[bank]);
            for (u32 row=0;row<3u;++row) {
                hide_text(frame,plain_row(bank,row),"");
                if (row!=Base) hide_text(frame,red_row(bank,row),"");
            }
            continue;
        }
        const bool focused=bank==bank_focus;
        const char* title=shift_numeric ? "SHIFT" : (m1 ?
            (bank==0u ? "Settings 1" : "Settings 2") :
            (m3 ? "M3" : bank_title[bank]));
        if (focused) {
            hide_text(frame,ordinary_bank(bank),title);
            place_text(frame,inverted_bank(bank),17,bank_y[bank],title);
        } else {
            place_text(frame,ordinary_bank(bank),17,bank_y[bank],title);
            hide_text(frame,inverted_bank(bank),title);
        }
        for (u32 row=0;row<3u;++row) {
            char contents[24]={};
            row_string(contents,source,bank,row);
            const bool selected=focused && source.edit_scene<Unknown && row==static_cast<u32>(source.edit_scene)*2u;
            if (selected) hide_text(frame,plain_row(bank,row),contents);
            else place_text(frame,plain_row(bank,row),106,row_y[bank][row],contents);
            if (row != Base) {
                if (selected) place_text(frame,red_row(bank,row),106,row_y[bank][row],contents);
                else hide_text(frame,red_row(bank,row),contents);
            }
        }
    }
    // Show genuine restoration warnings even while SHIFT is held. The
    // development-only raw-knob / stock-setter readout is not a V1 UI feature.
    // The old renderer signature is retained to avoid expanding API changes.
    (void)shift_stock_setter_index;
    (void)shift_raw_knob;
    (void)shift_chord;
    const unsigned fault=(morph_meter>>20u)&255u;
    const char* warning=fault==3u?"READBACK FAILED":fault==4u?"CONTEXT RESET":
        (fault==2u || fault==5u)?"WRITE REFUSED":fault?"MORPH FAULT":(morph_meter&0x000c0000u)==0x000c0000u ? "BASE INCOMPLETE" :
        (morph_meter&0x00080000u) ? "BASE PARTIAL" :
        (morph_meter&0x00040000u) ? "PARAM SKIPPED" : "";
    if(*warning) place_text(frame,shift_diagnostic,240,13,warning);
    else hide_text(frame,shift_diagnostic,"");
    // Old huge A/B letter is superseded by the right-hand wireframe meter.
    hide_text(frame,audible_letter,"");
    // Dynamic text path fixes labels that were absent from the v1.3.0e
    // hardware HUD when supplied only as unregistered static SVG text.
    place_text(frame,meter_letter_a,405,73,"A");
    place_text(frame,meter_letter_b,405,160,"B");
    draw_meter(frame,morph_meter);
    for(u32 k=0;k<4u;++k) frame.header[k][0]=0;
}

struct Gate {
    bool on_instrument_m2;
    bool same_physical_track_key_held;
    bool shift_held;
    bool project_loading;
    bool morph_screen_registered;
    bool menu_actions_preserved;
};
inline bool may_show(const Gate& g) {
    return g.on_instrument_m2 && g.same_physical_track_key_held && !g.shift_held &&
        !g.project_loading && g.morph_screen_registered && g.menu_actions_preserved;
}
} // namespace morph_hud
