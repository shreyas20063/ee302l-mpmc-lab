; Task 3 - traffic light controller, table-driven
; table at 2000H: pairs of (light byte, seconds)
; port 20H: D0 red, D1 yellow, D2 green, active high
; a real controller loops forever; here it runs the sequence twice and
; stops, so the simulator's run actually ends and shows the final state

# ORG 7000H

LXI SP,0FFFFH
MVI A,02H	; how many full cycles to run
STA 2100H
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
LDA 2100H	; one full cycle done
DCR A
STA 2100H
JNZ START
RST 1

; B seconds = call the 1-second delay B times
DELB: CALL DEL1S
DCR B
JNZ DELB
RET

; ~1 second at 1 MHz is LXI D,0A2C3H - shortened for the simulator,
; same reason as task 2
; uses DE so it cannot disturb B (seconds) or C (signal count)
DEL1S: LXI D,0010H
D1: DCX D
MOV A,D
ORA E
JNZ D1
RET

; green 04H for 5 s, yellow 02H for 2 s, red 01H for 5 s
# ORG 2000H
# DB 04H, 05H, 02H, 02H, 01H, 05H
