; Lab 1 Task 2 - difference = reference - measured
; inputs: 7501H (ref), 7502H (meas)   result: 7503H
; CY = 1 means borrow, result is then the two's complement of the magnitude

# ORG 7000H

LXI H,7501
MOV A,M
INX H
SUB M
INX H
MOV M,A
RST 1

; 40H - 2EH = 12H, no borrow. swap the bytes to see CY = 1 (EEH = -12H)
# ORG 7501H
# DB 40H, 2EH
