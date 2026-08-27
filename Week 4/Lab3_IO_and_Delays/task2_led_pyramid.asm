; Task 2 - inverted LED pyramid on four output ports, 1 s between rows
; ports assumed 01H-04H (handout leaves them unspecified)
; rows: FE = 1111.1110, 7C = 0111.1100, 38 = 0011.1000, 10 = 0001.0000
; ports latch their value, so earlier rows stay lit on their own

# ORG 7000H

LXI SP,0FFFFH	; CALL needs a working stack
MVI A,0FEH
OUT 01H
CALL DELAY
MVI A,7CH
OUT 02H
CALL DELAY
MVI A,38H
OUT 03H
CALL DELAY
MVI A,10H
OUT 04H
RST 1

; ~1 second at 1 MHz: loop body is 24 T-states,
; 1000000 / 24 = 41667 = A2C3H iterations
DELAY: LXI B,0A2C3H
DLOOP: DCX B
MOV A,B		; DCX sets no flags, so test BC by hand
ORA C
JNZ DLOOP
RET
