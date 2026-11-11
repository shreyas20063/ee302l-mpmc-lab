; Task 1 - copy ADC samples >= 80H from 2000H block to 3000H block
; ten samples at 2000H-2009H, crossings to 3000H+, count to 4000H

# ORG 7000H

LXI H,2000H	; source pointer
LXI D,3000H	; destination pointer
MVI C,0AH	; ten samples to examine
MVI B,00H	; crossing count
LOOP: MOV A,M
CPI 80H		; A < 80H sets CY
JC SKIP
XCHG		; HL = destination, DE = source (the simulator's STAX D is broken)
MOV M,A		; store crossing at destination
INX H
XCHG		; swap back: HL = source, DE = next free slot
INR B
SKIP: INX H
DCR C
JNZ LOOP
MOV A,B
STA 4000H
RST 1

; crossings here: 85,80,FF,90,A5 -> 3000H-3004H, count 05H at 4000H
; 7FH and 80H are the boundary pair: only 80H qualifies
# ORG 2000H
# DB 12H, 85H, 7FH, 80H, 03H, 0FFH, 45H, 90H, 7EH, 0A5H
