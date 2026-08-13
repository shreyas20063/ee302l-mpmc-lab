; Pt-51 LED blink - LEDs on P1.4-P1.7, switches on P1.0-P1.3
; sdas8051 (ASxxxx) syntax - NOT the same dialect as MCU 8051 IDE on lab PCs

	.module blink

; linker wants these areas declared even when empty
	.area HOME (CODE)
	.area XSEG (XDATA)
	.area PSEG (XDATA)

P1 = 0x90

	.area CODE (ABS)
	.org 0x0000
	ljmp main

	.org 0x0100
main:
	mov P1, #0xF0
	acall delay
	mov P1, #0x00
	acall delay
	sjmp main

delay:
	mov r2, #255
delay1:
	mov r1, #255
here:
	djnz r1, here
	djnz r2, delay1
	ret
