; Q3: 2-digit BCD counter on port 01H, paused by RST 7.5 which blinks port 02H 5 times
; Main code 2000H, ISR at 003CH, stack at 2700H, count kept in register C
# ORG 2000H
LXI SP,2700H
MVI C,00H
MVI A,08H ; MSE=1, unmask RST 7.5
SIM
EI
MAIN:MOV A,C
OUT 01H
CALL DELAY
MOV A,C
CPI 99H ; wrap 99 -> 00 by hand (simulator DAA gets this case wrong)
JNZ NEXT
MVI C,00H
JMP MAIN
NEXT:ADI 01H
DAA
MOV C,A
JMP MAIN
DELAY:MVI D,10H ; ~4000 loops, sized for the simulator (real 3 MHz 8085 needs ~100x more)
DLY1:MVI E,0FFH
DLY2:DCR E
JNZ DLY2
DCR D
JNZ DLY1
RET
# ORG 003CH
ISR:PUSH PSW
PUSH B ; count (C) saved on stack
PUSH D
MVI B,05H
BLINK:MVI A,0FFH
OUT 02H
CALL DELAY
MVI A,00H
OUT 02H
CALL DELAY
DCR B
JNZ BLINK
WAIT:MVI A,18H ; reset the RST 7.5 flip-flop
SIM
MVI A,08H ; simulator keeps the reset bit until written back to 0
SIM
RIM
ANI 40H ; wait until RST 7.5 input is released
JNZ WAIT
POP D
POP B ; count restored into C
POP PSW
LXI SP,2700H ; simulator does not push a return address on interrupt, so no RET
EI
JMP MAIN