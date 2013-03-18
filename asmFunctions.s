    .text
    .global _readADC

_readADC:
    MOV 0, W0
	MOV ADCBUF0, W1
    RETURN

.global _round_Q16_2_int

_round_Q16_2_int:
		MOV W1, W0
		RETURN

.end



