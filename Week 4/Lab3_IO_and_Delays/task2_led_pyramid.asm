; Task 2 - inverted LED pyramid in the I/O port grid, 01 = LED on
; COMPUTED version: no pattern table. The only data is the seed row FEH;
; every next row = (prev << 1) AND (prev >> 1), which erodes one LED from
; each end (row widths 7,5,3,1). Cells are peeled off the row byte one
; bit at a time with RLC.
; D = current row byte, E = rotating copy during extraction

# ORG 7000H

LXI SP,0FFFFH
MVI D,0FEH	; seed: top row, seven LEDs lit

MOV E,D		; row 0 -> ports 00-07
CALL GETBIT
OUT 00H
CALL GETBIT
OUT 01H
CALL GETBIT
OUT 02H
CALL GETBIT
OUT 03H
CALL GETBIT
OUT 04H
CALL GETBIT
OUT 05H
CALL GETBIT
OUT 06H
CALL GETBIT
OUT 07H
CALL DELAY
MOV A,D
CALL NEXTROW
MOV D,A

MOV E,D		; row 1 -> ports 10-17
CALL GETBIT
OUT 10H
CALL GETBIT
OUT 11H
CALL GETBIT
OUT 12H
CALL GETBIT
OUT 13H
CALL GETBIT
OUT 14H
CALL GETBIT
OUT 15H
CALL GETBIT
OUT 16H
CALL GETBIT
OUT 17H
CALL DELAY
MOV A,D
CALL NEXTROW
MOV D,A

MOV E,D		; row 2 -> ports 20-27
CALL GETBIT
OUT 20H
CALL GETBIT
OUT 21H
CALL GETBIT
OUT 22H
CALL GETBIT
OUT 23H
CALL GETBIT
OUT 24H
CALL GETBIT
OUT 25H
CALL GETBIT
OUT 26H
CALL GETBIT
OUT 27H
CALL DELAY
MOV A,D
CALL NEXTROW
MOV D,A

MOV E,D		; row 3 -> ports 30-37
CALL GETBIT
OUT 30H
CALL GETBIT
OUT 31H
CALL GETBIT
OUT 32H
CALL GETBIT
OUT 33H
CALL GETBIT
OUT 34H
CALL GETBIT
OUT 35H
CALL GETBIT
OUT 36H
CALL GETBIT
OUT 37H
RST 1

; next bit of the row -> A as 01H or 00H
; RLC pushes E's leftmost bit into CY, MVI leaves CY alone, ACI adds it
GETBIT: MOV A,E
RLC
MOV E,A
MVI A,00H
ACI 00H
RET

; A = current row -> A = next row: (A << 1) AND (A >> 1)
; 8085 has no plain shifts: clear CY with ORA A, then rotate through CY
NEXTROW: MOV B,A
ORA A
RAR		; A = row >> 1
MOV C,A
MOV A,B
ORA A
RAL		; A = row << 1
ANA C		; keep only bits lit in BOTH shifts
RET

; row-to-row pause (0200H is watchable at Normal speed)
DELAY: LXI B,0A00H
DLOOP: DCX B
MOV A,B
ORA C
JNZ DLOOP
RET
