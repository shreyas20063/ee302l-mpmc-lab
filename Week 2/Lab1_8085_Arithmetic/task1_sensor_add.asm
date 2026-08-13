; Lab 1 Task 1 - add two 8-bit sensor readings
; inputs: 7501H, 7502H   result: 7503H

# ORG 7000H

LXI H,7501
MOV A,M
INX H
ADD M
INX H
MOV M,A
RST 1

; 19H + 1CH = 35H
# ORG 7501H
# DB 19H, 1CH
