; Task 2 visual variant - draws the pyramid ACROSS the port grid,
; one port box per LED, 01 = on. Grid rows 00/10/20/30 become the
; four pyramid rows. Demo only - the graded answer stays
; task2_led_pyramid.asm (spec wires 8 LEDs to one port's byte).

# ORG 7000H

LXI SP,0FFFFH
MVI A,01H

OUT 00H		; row 1: seven on, ports 00-06
OUT 01H
OUT 02H
OUT 03H
OUT 04H
OUT 05H
OUT 06H
CALL DELAY

OUT 11H		; row 2: five on, centered, ports 11-15
OUT 12H
OUT 13H
OUT 14H
OUT 15H
CALL DELAY

OUT 22H		; row 3: three on, ports 22-24
OUT 23H
OUT 24H
CALL DELAY

OUT 33H		; row 4: one on, port 33
RST 1

; 1 s at 1 MHz would be LXI B,0A2C3H - short count for the simulator
DELAY: LXI B,0010H
DLOOP: DCX B
MOV A,B
ORA C
JNZ DLOOP
MVI A,01H	; delay clobbers A, restore the 01 for the next OUTs
RET
