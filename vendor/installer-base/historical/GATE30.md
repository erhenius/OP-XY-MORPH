# Gate30 hardening

Retries failed scramble S refreshes within the existing session; blocks AUX actions until refresh succeeds and discards gestures arriving during refresh. Context reset clears pending work. AUX and HUD now share movement authorization through both S verification passes. Removes a duplicated controller slot map and obsolete introductory comments.

Gate29 and Test3 hardware acceptance applies to the previous revision only. Gate30 requires a fresh native build and hardware acceptance. No new CPU timing or REC-held automation claim. The oscillator scaffold is retained because its generated ABI role has not been qualified for removal.

Regression coverage: transient scramble-read failure followed by recovery; stale movement discarded during recovery; AUX movement during return-to-S verification. Existing context, snapshot, input ownership and scene tests remain included.
