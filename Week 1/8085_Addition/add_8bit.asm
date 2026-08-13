; add two 8-bit numbers
; inputs: 7501H, 7502H   result: 7503H

# ORG 7000H

LXI H,7501
MOV A,M
INX H
ADD M
INX H
MOV M,A
RST 1

; 12H + 13H = 25H
# ORG 7501H
# DB 12H, 13H
