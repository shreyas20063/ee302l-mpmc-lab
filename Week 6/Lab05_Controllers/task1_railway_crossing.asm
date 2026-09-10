; Task 1 - railway crossing controller
; port 80H train sensor in (01 = present), port 82H gate out (01 = closed)
; train count at 9000H, main at 7000H, ISR at 003CH, stack at 2700H

# ORG 7000H
DI		; nothing may fire until the stack is ready
LXI SP,2700H
MVI A,00H
OUT 82H		; gate open to begin with
MAIN: CALL MONITOR	; MONITOR never returns on its own, only through the ISR's RET
JMP MAIN
MONITOR: MVI A,08H	; unmask RST 7.5 (also clears a leftover reset bit)
SIM
EI		; enabled only now, with a return address already on the stack
IDLE: JMP IDLE	; infinite monitoring loop

; 1 s at 1 MHz would be LXI D,0A2C3H (24 T-states a lap) - short count for the simulator
DELAY: LXI D,0400H
DL: DCX D
MOV A,D
ORA E
JNZ DL
RET

; RST 7.5 - a train has been detected
# ORG 003CH
PUSH PSW
PUSH B
PUSH D
LDA 9000H
INR A
STA 9000H	; one more train
MVI A,01H
OUT 82H		; gate closed
CALL DELAY
MVI B,0AH	; poll the sensor up to 10 times, then assume the train has passed
WAIT: IN 80H	; stay closed while the sensor still shows a train
ORA A
JZ GONE
CALL DELAY
DCR B
JNZ WAIT
GONE: MVI A,00H
OUT 82H		; gate open
MVI A,18H	; reset the 7.5 flip-flop, then release it
SIM
MVI A,08H
SIM
ARM: RIM	; wait for the interrupt line to drop before going back
ANI 40H
JNZ ARM
POP D
POP B
POP PSW
EI
RET

# ORG 9000H
# DB 00H
