;This is the code for adding two one byte values which are from consecutive memory locations to the next location

# ORG 7000H

LXI H,7501
MOV A,M
INX H
ADD M
INX H
MOV M,A
RST 1

# ORG 7501H
# DB 19H, 1CH
