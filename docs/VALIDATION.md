# v1.1.0 validation

Gate36a: owner reports no regressions and correct MIDI feature operation on hardware. This is owner acceptance, not measured CPU usage or comprehensive edge-case qualification.

Local checks: 17,105 controller/SDK/display integration assertions; stock recorder/MIDI/hardware functions use host models. 33 manager tests pass with the external private fixture. SDK track-control: 17 tests pass with private stock input; includes a host leading-underscore ABI model. The source patch integrity/refusal/idempotence checks pass.

Full SDK suite: 546 tests, 18 failures, 15 errors, 38 skips. NOT GREEN. See builder/SDK_CHECK_GATE36a.log and builder/CHECK_RESULTS.md. Blocked fixture/path/history checks are not successful backup or rollback qualification.

Owner native log from the earlier Gate36 attempt reached GNU link parity with 3,803,576 allocated bytes, then failed hook resolution. Gate36a corrects the explicit export name. That earlier figure is not a verified final Gate36a allocation. Every final build must perform its own allocation and reconstruction checks.

Public builder selects the owner's matching private kit for authenticated runtime objects. No personal kit, firmware, project or compiled collection is distributed. The public packaging path is host-tested; fresh Mac/device acceptance using this exact public archive remains pending. CPU profiling and upstream catalog approval remain pending. Packaging does not stage a device.

Exact public archive host checks: integrity verifier PASS; 17,105 integration assertions PASS; M1 and route parity PASS; manager suite 33 tests, seven explicitly skipped without private runtime fixture, remaining 26 PASS.
