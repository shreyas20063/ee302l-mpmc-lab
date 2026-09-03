; Q3: 2-digit BCD counter on port 01H, RST 7.5 pauses it and blinks port 02H 5 times
; Main program at 2000H, ISR at 003CH, stack at 2700H, count kept in register B

# ORG 2000H
LXI SP,2700H
MVI A,08H	; MSE=1, unmask RST 7.5
SIM
MVI B,00H
EI
MAIN: CALL TICK
JMP MAIN

; show B, wait, then B = B + 1 in BCD
TICK: MOV A,B
OUT 01H
CALL DELAY
MOV A,B
CPI 99H
JZ ZERO	; 99 -> 00 done by hand, DAA carry is unreliable here
ADI 01H
DAA
MOV B,A
RET
ZERO: MVI B,00H
RET

; 1000H is for the simulator, about 7A00H gives 0.25 s on a 3 MHz 8085
DELAY: LXI D,1000H
DLOOP: DCX D
MOV A,D
ORA E
JNZ DLOOP
RET

; RST 7.5 service routine
# ORG 003CH
PUSH PSW
PUSH B	; count goes on the stack
PUSH D
MVI C,05H
BLINK: MVI A,0FFH
OUT 02H
CALL DELAY
MVI A,00H
OUT 02H
CALL DELAY
DCR C
JNZ BLINK
MVI A,18H	; reset the RST 7.5 flip-flop
SIM
MVI A,08H	; simulator keeps bit 4 set, so write it back to 0
SIM
WAIT: RIM	; simulator holds the 7.5 line high till the cell is toggled off
ANI 40H
JNZ WAIT
POP D
POP B	; count comes back
POP PSW
EI
RET
