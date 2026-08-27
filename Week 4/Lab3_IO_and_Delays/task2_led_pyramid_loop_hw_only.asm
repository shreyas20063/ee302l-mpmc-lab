; Task 2, loop version - KEPT FOR THE RECORD, real hardware only.
; This is the proper data-driven loop (one OUT, port patched from data).
; Works on a real 8085; this simulator executes its pre-assembled
; listing, so the patch is ignored - tested 27/08, see README.
; pattern is DATA: port list at 2000H, FF = end of row (delay), FE = end
; 8085's OUT takes the port only as an immediate, so the loop writes each
; port number into the OUT instruction's operand byte at 7051H before
; running it (self-modifying code - the standard 8085 answer to this)

# ORG 7000H

LXI SP,0FFFFH
LXI H,2000H
LOOP: MOV A,M
INX H
CPI 0FEH
JZ DONE
CPI 0FFH
JZ ROWEND
STA 7051H	; patch the port number into the OUT below
MVI A,01H
CALL POKE
JMP LOOP
ROWEND: CALL DELAY
JMP LOOP
DONE: RST 1

# ORG 7050H
POKE: OUT 00H	; operand byte at 7051H is rewritten before every call
RET

; 1 s at 1 MHz would be LXI B,0A2C3H - short count for the simulator
DELAY: LXI B,0010H
DLOOP: DCX B
MOV A,B
ORA C
JNZ DLOOP
RET

; the pyramid itself: port = grid cell (high nibble row, low nibble col)
# ORG 2000H
# DB 00H,01H,02H,03H,04H,05H,06H,0FFH
# DB 11H,12H,13H,14H,15H,0FFH
# DB 22H,23H,24H,0FFH
# DB 33H,0FEH
