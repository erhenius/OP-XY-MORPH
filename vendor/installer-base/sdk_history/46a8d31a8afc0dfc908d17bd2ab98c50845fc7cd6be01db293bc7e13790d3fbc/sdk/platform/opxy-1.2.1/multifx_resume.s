// Exact displaced entry at 8c0b5d58, then JUMP.A 8c0b5d60.
// LINK 0; [--SP] = (R7:7, P5:5); P5 = R0. No PC-relative original.
// The build independently authenticates all sixteen bytes and symbol bounds.
.section .text
.align 2
.global sdk_multifx_stock_event
.type sdk_multifx_stock_event, STT_FUNC
sdk_multifx_stock_event:
.byte 0x00,0xe8,0x00,0x00,0xfd,0x05,0x28,0x32
.byte 0x00,0xdc,0x0b,0x8c,0x60,0x5d,0x00,0x00
.size sdk_multifx_stock_event, .-sdk_multifx_stock_event
