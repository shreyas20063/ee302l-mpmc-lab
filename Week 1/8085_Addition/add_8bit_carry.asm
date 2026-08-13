; add two 8-bit numbers, keep the carry
; inputs: 7501H, 7502H   result: 7503H (sum), 7504H (carry)

# ORG 7000H

LXI H,7501
MVI C,00H
MOV A,M
INX H
ADD M
JNC SKIP
INR C
SKIP: INX H
MOV M,A
INX H
MOV M,C
RST 1

; 88H + 88H = 110H -> 7503=10H, 7504=01H
# ORG 7501H
# DB 88H, 88H
