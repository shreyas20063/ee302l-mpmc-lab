; Task 3 - traffic light controller, table-driven, repeats forever
; table at 2000H: pairs of (light byte, seconds)
; port 20H: D0 red, D1 yellow, D2 green, active high
; no RST 1 - a controller never stops; use the simulator's Stop button

# ORG 7000H

LXI SP,0FFFFH
START: LXI H,2000H
MVI C,03H	; three signals in the sequence
NEXT: MOV A,M	; light pattern
OUT 20H
INX H
MOV B,M		; how many seconds it stays on
INX H
CALL DELB
DCR C
JNZ NEXT
JMP START	; sequence over, begin again

; B seconds = call the 1-second delay B times
DELB: CALL DEL1S
DCR B
JNZ DELB
RET

; ~1 second at 1 MHz, same 24 T-state loop as task 2
; uses DE so it cannot disturb B (seconds) or C (signal count)
DEL1S: LXI D,0A2C3H
D1: DCX D
MOV A,D
ORA E
JNZ D1
RET

; green 04H for 5 s, yellow 02H for 2 s, red 01H for 5 s
# ORG 2000H
# DB 04H, 05H, 02H, 02H, 01H, 05H
