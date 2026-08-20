; Exp 1 - total components = batch size x batches, by repeated addition
; 2500H batch size, 2501H batches, 2502H result LSB, 2503H result MSB

# ORG 7000H

LXI H,0000
MVI D,00H
LDA 2500H
MOV E,A		; DE = batch size
LDA 2501H
ORA A		; zero batches -> total stays 0
JZ STORE
MOV C,A
LOOP: DAD D	; total += batch size
DCR C
JNZ LOOP
STORE: SHLD 2502H
RST 1

; 40H x 05H = 320 = 0140H -> 2502=40H, 2503=01H
# ORG 2500H
# DB 40H, 05H
