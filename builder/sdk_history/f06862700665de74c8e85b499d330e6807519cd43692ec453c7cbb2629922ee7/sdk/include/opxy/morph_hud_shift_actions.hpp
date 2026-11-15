// XY Morph AUX Stage 2 — HUD-owned SHIFT K3/K4 press arbitration.
// Source-only state machine. No stock calls, no parameter writes, no timing
// assumptions, no new SDK ABI. Single gestures commit on RELEASE so a chord
// always wins while its keys actually overlap. A chord commits once when
// both keys are down, and cannot also commit either single on their release.
#pragma once
#include <stdint.h>
namespace morph_hud_shift_actions {
struct Router {
    enum { A=1u, B=2u, LIVE_BYPASS=4u };
    uint32_t previous;
    uint32_t first;
    uint32_t pending;
    uint32_t suppressed;  // swallow late stock K3/K4 clicks after SHIFT
    uint32_t outside_previous;
    bool primed;
    bool ignore_until_clear;
    bool chord_fired;

    void reset() {
        previous=first=pending=suppressed=outside_previous=0u;
        primed=ignore_until_clear=chord_fired=false;
    }
    // Call once per compositor route observation; authorize ONLY when page 219
    // is actually substituted, the Morph owner/track/SHIFT gate is live,
    // and the Track-first SHIFT chord is active. Physical mask: 1=K3, 2=K4.
    void observe(bool authorized, bool shift_down, uint32_t raw_mask) {
        const uint32_t now=raw_mask&3u;
        if (!authorized) {
            // No action may survive loss of physical HUD ownership/visibility.
            pending=first=previous=0u;
            primed=ignore_until_clear=chord_fired=false;
            // A genuine fresh, non-SHIFT press starts a new stock gesture.
            // A key carried across SHIFT release is NOT a fresh press.
            if (!shift_down) suppressed &= ~(now & ~outside_previous);
            outside_previous=now;
            return;
        }
        outside_previous=now;
        // Hold tombstones until stock's delayed click is either swallowed or
        // a new genuine unshifted physical press begins.
        suppressed |= now;
        if (!primed) {
            primed=true;
            previous=now;
            // Do not reinterpret an already-held button as a new SHIFT push.
            ignore_until_clear=(now!=0u);
            return;
        }
        if (ignore_until_clear) {
            if (!now) ignore_until_clear=false;
            previous=now;
            return;
        }
        if (chord_fired) {
            if (!now) chord_fired=false;
            previous=now;
            return;
        }
        const uint32_t rise=now & ~previous;
        const uint32_t fall=previous & ~now;
        if (now==3u && (rise || first)) {
            pending=LIVE_BYPASS;
            first=0u;
            chord_fired=true;
        } else {
            if (!first && (rise==1u || rise==2u)) first=rise;
            if (first && (fall&first)) {
                // If second press is already down but first no longer is,
                // the order is ambiguous; fail closed instead of jumping.
                if (!now) pending=(first==1u ? A : B);
                first=0u;
            }
        }
        previous=now;
    }
    uint32_t take(bool authorized) {
        const uint32_t action=authorized ? pending : 0u;
        pending=0u;
        return action;
    }
    // Called only after a LIVE HUD ownership check. Never route a late SHIFT
    // K3 click into ordinary SET BASE, even if SHIFT was released first.
    bool swallow_late_click(uint32_t encoder, bool shift_down) {
        if (encoder!=2u && encoder!=3u) return false;
        const uint32_t bit=1u<<(encoder-2u);
        if (!(suppressed & bit)) return false;
        if (!shift_down) suppressed &= ~bit;
        return true;
    }
};
} // namespace morph_hud_shift_actions
