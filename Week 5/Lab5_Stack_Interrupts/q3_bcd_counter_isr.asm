; Q3 - 2-digit BCD counter on port 01H; RST 7.5 pauses it, blinks port 02H
; five times, then counting resumes from where it stopped
; main at 2000H, ISR at 003CH (the RST 7.5 vector), stack at 2700H
; B holds the count. The ISR saves it on the stack and hands it back untouched

# ORG 2000H

LXI SP,2700H
MVI A,08H	; MSE set, RST 7.5 unmasked (same byte as the handout example)
SIM
EI
MVI B,00H
COUNT: MOV A,B
OUT 01H
CALL DELAY
MOV A,B
ADI 01H
DAA		; keep it decimal: 09 -> 10, 99 -> 00
MOV B,A
JMP COUNT

DELAY: LXI D,0200H
DL: DCX D
MOV A,D
ORA E
JNZ DL
RET

; interrupt service routine - the CPU lands here when RST 7.5 fires
# ORG 003CH
PUSH PSW
PUSH B		; the count, safe on the stack
PUSH D
MVI C,05H	; five blinks
BLINK: MVI A,0FFH
OUT 02H
CALL DELAY
MVI A,00H
OUT 02H
CALL DELAY
DCR C
JNZ BLINK
POP D
POP B		; count back exactly where it was
POP PSW
EI		; interrupts got switched off on entry, switch them back on
RET
