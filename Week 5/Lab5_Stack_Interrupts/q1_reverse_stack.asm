; Q1 - reverse five bytes at 2050H in place, using the stack
; push them in order, pop them back into the same slots:
; the last one pushed is the first one popped, so the order flips

# ORG 2000H

LXI SP,2700H
LXI H,2050H
MVI D,05H
PUSHL: MOV B,M
PUSH B		; B carries the byte, C rides along as junk
INX H
DCR D
JNZ PUSHL
LXI H,2050H	; back to the start, now write while popping
MVI D,05H
POPL: POP B
MOV M,B
INX H
DCR D
JNZ POPL
RST 1

; 11 22 33 44 55 -> 55 44 33 22 11
# ORG 2050H
# DB 11H, 22H, 33H, 44H, 55H
