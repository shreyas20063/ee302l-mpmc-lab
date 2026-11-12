; Lab 1 Task 3 - add two 16-bit counts, low byte stored first
; count1: 7601-02H  count2: 7603-04H  sum: 7500-01H  carry out: 7502H

# ORG 7000H

LDA 7601H
MOV B,A
LDA 7603H
ADD B
STA 7500H
LDA 7602H
MOV B,A
LDA 7604H
ADC B		; carry from the low bytes rides in here
STA 7501H
MVI A,00H
JNC DONE
INR A
DONE: STA 7502H
RST 1

; 80FFH + 9001H = 11100H -> 7500=00H, 7501=11H, 7502=01H
# ORG 7601H
# DB FFH, 80H, 01H, 90H
