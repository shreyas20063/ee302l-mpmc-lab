; Exp 3 - pressure reading vs safe window
; 2500H reading, 2501H min, 2502H max
; 2503H status: 01 = LOW, 02 = HIGH, 00 = NORMAL (min <= reading <= max)

# ORG 7000H

LDA 2501H
MOV B,A		; B = min
LDA 2502H
MOV C,A		; C = max
LDA 2500H	; A = reading
CMP B
JC LOW		; reading < min
CMP C
JC OK		; reading < max
JZ OK		; reading = max
MVI A,02H	; reading > max
JMP STORE
OK: MVI A,00H
JMP STORE
LOW: MVI A,01H
STORE: STA 2503H
RST 1

; 50H inside [30H, 70H] -> status 00
; edit 2500H to 20H -> 01, to 90H -> 02, to 30H or 70H -> 00
# ORG 2500H
# DB 50H, 30H, 70H
