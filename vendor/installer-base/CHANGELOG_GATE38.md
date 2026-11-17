# Gate38 sensitivity

M3 K2 adjusts movement-rate gain from -90% to +90% in 1% steps relative to hardware-accepted Gate37a. Default 0%; K2 clicks have no action. -90% = 0.1x speed; +90% = 1.9x speed. Applies during movement without a position jump. PITCH behavior and stock calibration are unchanged.

Version 4 saves sensitivity in existing flag space with bounds validation; older banks load at 0%. Saved payload remains 5684 bytes. One new 4-byte static setting. Native allocation unmeasured. Older releases reject v4 banks; preserve original project companions for rollback.

47 pad checks and 17,342 integration assertions pass, including gain, endpoints, K2, save/load. M1 and route parity pass. SDK/profile/hooks unchanged; historical full SDK failures remain documented. Native compilation and device acceptance pending. Gate37a preserved as rollback. Nothing staged.
