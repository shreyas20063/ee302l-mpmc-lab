; Task 2 - tank level controller
; ten readings 9000H-9009H (00-0AH), pump at 2000H, alarm at 2001H
; counts: low 3000H, normal 3001H, high 3002H, average 3003H
; code at 7000H, stack at 2700H. Overflow alarm raised with RST 7 (software interrupt, vector 0038H)

# ORG 7000H
LXI SP,2700H
MVI A,00H
STA 2000H	; pump reset to zero
STA 2001H	; alarm off
START: LXI H,9000H
MVI B,00H	; low count
MVI C,00H	; normal count
MVI D,00H	; high count
MVI E,00H	; running sum
NEXT: MOV A,M
ADD E
MOV E,A
MOV A,M
CPI 03H
JC LOW
CPI 09H
JC NORMAL
INR D		; high: pump off
MVI A,00H
STA 2000H
JMP DONE
LOW: INR B	; low: pump on
MVI A,01H
STA 2000H
JMP DONE
NORMAL: INR C	; normal: pump stays as it is
DONE: INX H
MOV A,L
CPI 0AH		; ten samples done when L reaches 0A
JNZ NEXT
MOV A,B
STA 3000H
MOV A,C
STA 3001H
MOV A,D
STA 3002H
MOV A,E		; average = sum / 10 by repeated subtraction
MVI B,00H
DIV: CPI 0AH
JC AVG
SUI 0AH
INR B
JMP DIV
AVG: MOV A,B
STA 3003H
MOV A,E
CPI 51H		; sum > 80 means average > 80% of full scale (0AH)
JC START
RST 7		; overflow: raise the interrupt
JMP START

; 1 s at 1 MHz would be LXI D,0A2C3H - short count for the simulator
DELAY: LXI D,0400H
DL: DCX D
MOV A,D
ORA E
JNZ DL
RET

; overflow protection: pump off, alarm for a second, back to monitoring
ALARM: PUSH PSW
PUSH D
MVI A,00H
STA 2000H
MVI A,0FFH
STA 2001H
CALL DELAY
MVI A,00H
STA 2001H
POP D
POP PSW
RET

# ORG 0038H
JMP ALARM

; low 02,01,00  normal 05,07,03,08,06  high 09,0A  sum 51 -> average 05, no alarm
# ORG 9000H
# DB 02H,05H,09H,01H,07H
# ORG 9005H
# DB 0AH,03H,08H,00H,06H
