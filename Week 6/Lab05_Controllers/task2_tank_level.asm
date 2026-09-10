; Task 2 - ten tanks, ten pumps, tanks joined by an overflow pipe on top
; readings 9000H-9009H (00-0AH), one per tank
; pump states 2002H-200BH, one per tank (01 on / 00 off); 2000H = pump of the tank being processed
; 2001H = alarm (FF on / 00 off)
; counts: low 3000H, normal 3001H, high 3002H; 3003H = running average of the tanks read so far
; average above 80% of full scale (0AH) after any tank raises the overflow interrupt (RST 7, vector 0038H):
; every pump off, alarm for a second, then monitoring continues
; code at 7000H, stack at 2700H

# ORG 7000H
LXI SP,2700H
CALL ALLOFF	; every pump off to begin with
MVI A,00H
STA 2001H	; alarm off
START: MVI A,00H
STA 3000H
STA 3001H
STA 3002H
LXI H,9000H
MVI E,00H	; running sum
NEXT: MOV A,M
ADD E
MOV E,A		; sum of the tanks read so far
MVI B,20H	; BC -> this tank's pump entry, 2002H + tank index
MOV A,L
ADI 02H
MOV C,A
MOV A,M
CPI 03H
JC LOW
CPI 09H
JC NORMAL
LDA 3002H	; high: count it, this tank's pump off
INR A
STA 3002H
MVI A,00H
STAX B
STA 2000H
JMP AVG
LOW: LDA 3000H	; low: count it, this tank's pump on
INR A
STA 3000H
MVI A,01H
STAX B
STA 2000H
JMP AVG
NORMAL: LDA 3001H	; normal: count it, this tank's pump keeps its own state
INR A
STA 3001H
LDAX B
STA 2000H
AVG: MOV A,L	; tanks read so far = L + 1
INR A
MOV C,A
MOV A,E		; running average = sum / tanks, by repeated subtraction
MVI B,00H
DIV: CMP C
JC SAVE
SUB C
INR B
JMP DIV
SAVE: MOV A,B
STA 3003H
MOV A,C		; 8 x tanks = the 80% line for that many tanks
RLC
RLC
RLC
CMP E		; sum above the line means average > 80%
JNC DONE
RST 7		; overflow: raise the interrupt
DONE: INX H
MOV A,L
CPI 0AH		; ten tanks done when L reaches 0A
JNZ NEXT
JMP START	; and again, forever

; all ten pumps off: clear the table and the current-pump output
ALLOFF: LXI H,2002H
MVI B,0AH
OFF1: MVI M,00H
INX H
DCR B
JNZ OFF1
MVI A,00H
STA 2000H
RET

; 1 s at 1 MHz would be LXI D,0A2C3H - short count for the simulator
DELAY: LXI D,0400H
DL: DCX D
MOV A,D
ORA E
JNZ DL
RET

; overflow protection: every pump off, alarm for a second, back to monitoring
ALARM: PUSH PSW
PUSH B
PUSH D		; DELAY uses DE and E is the running sum
PUSH H		; ALLOFF walks the pump table with HL
CALL ALLOFF
MVI A,0FFH
STA 2001H
CALL DELAY
MVI A,00H
STA 2001H
POP H
POP D
POP B
POP PSW
RET

# ORG 0038H
JMP ALARM

; low 02,01,00  normal 05,07,03,08,06  high 09,0A  final sum 51 -> average 05, never above 80%
# ORG 9000H
# DB 02H,05H,09H,01H,07H
# ORG 9005H
# DB 0AH,03H,08H,00H,06H
