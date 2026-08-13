;-----------------------------------------------------------------------------
; Pt-51 (AT89C5131A) — blink all four onboard LEDs.
; ASxxxx syntax, for sdas8051 on macOS. NOT the same dialect as MCU 8051 IDE.
;
; P1 upper nibble (P1.4-P1.7) = LED1-LED4, lower nibble (P1.0-P1.3) = SW1-SW4,
; so #0xF0 drives the LEDs without disturbing the switch inputs.
;-----------------------------------------------------------------------------
	.module blink

; The linker expects these areas to exist even when unused; declaring them
; keeps sdcc from emitting "No definition of area HOME/XSEG/PSEG".
	.area HOME  (CODE)
	.area XSEG  (XDATA)
	.area PSEG  (XDATA)

P1 = 0x90			; Port 1 SFR

	.area CODE  (ABS)
	.org 0x0000
	ljmp	main

	.org 0x0100
main:
	mov	P1, #0xF0	; all four LEDs on
	acall	delay
	mov	P1, #0x00	; all four LEDs off
	acall	delay
	sjmp	main

; Nested down-counters. ~255*255 iterations of djnz; at 24 MHz this is a
; visible but fairly quick blink. Raise the outer count to slow it down.
delay:
	mov	r2, #255
delay1:
	mov	r1, #255
here:
	djnz	r1, here
	djnz	r2, delay1
	ret
