; Q2 - add 05H and 0AH in a subroutine; main keeps its own BC safe on the stack
; the subroutine uses B and C as scratch, so without PUSH/POP the caller's
; pair would come back wrecked. result at 2060H

# ORG 2000H

LXI SP,2700H
MVI B,11H	; main's own data, pretend it matters
MVI C,22H
PUSH B		; save the pair before the call
CALL ADD2
POP B		; pair back exactly as it was
RST 1

ADD2: MVI B,05H
MVI C,0AH
MOV A,B
ADD C
STA 2060H	; 05 + 0A = 0F
RET
