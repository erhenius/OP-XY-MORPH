# Gate37a

Owner reports Gate37 operates without regressions, except pad reversal from exact endpoints.

- Pad speed increased by exactly 50%: full calibrated pressure is 96 travel steps/second.
- Held HUD entry reduced from 250 to 150 ms.
- At exact A/B, inward fractional movement now accumulates. Only outward saturation discards the residual; light inward pressure no longer sticks.

44 pure pad checks pass, including both endpoint reversals and exact 96-step timing.
17,340 SDK/controller/display assertions pass. M1 and route parity pass.
No firmware profile or SDK input-hook change. Identified static storage and saved payload unchanged from Gate37. Native linked-size delta unmeasured.
Gate37a native compilation and device acceptance remain pending. No firmware staged.
