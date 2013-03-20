    .text
    .global _readADC

_readADC:
	MOV ADCBUF0, W0
    RETURN

.global _round_Q16_2_int

_round_Q16_2_int:
		MOV W1, W0
		RETURN

.global _I_controller_update

_I_controller_update:
	MOV #0x0001, W2
	MUL.SS W1, W2, W4
	ADD W0, W4, W0
	BRA OV, OVERFLOW
	RETURN
OVERFLOW:
	BRA N NEGATIVE
	MOV #0x8000, W0
	RETURN
NEGATIVE:
	MOV #0x7FFF, W0
	RETURN

.global _update_IIR_Controller

_update_IIR_Controller:
	MUL.SS W0,W1,W4
	ADD W2,W4,W0
	BRA OV, OVERFLOW_MAC
	RETURN
OVERFLOW_MAC:
	BRA N, NEG_MAC
	MOV #0x8000, W0
	RETURN
NEG_MAC:
	MOV #0x7FFF, W0
	RETURN

.end



