; Exp 2 - fuel efficiency = distance / fuel, by repeated subtraction
; 2500H distance, 2501H fuel, 2502H quotient, 2503H remainder
; fuel = 0 has no answer: store FFH as quotient, distance as remainder

# ORG 7000H

LDA 2501H
MOV B,A		; B = fuel
ORA A
JZ DIVZERO
LDA 2500H	; A = distance
MVI C,00H	; C = quotient
LOOP: CMP B	; A < fuel -> CY, division over
JC DONE
SUB B
INR C
JMP LOOP
DONE: STA 2503H	; what is left in A is the remainder
MOV A,C
STA 2502H
RST 1
DIVZERO: MVI A,0FFH
STA 2502H
LDA 2500H
STA 2503H
RST 1

; EAH / 0CH = 234 / 12 -> quotient 13H (19), remainder 06H
# ORG 2500H
# DB 0EAH, 0CH
