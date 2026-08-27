; Task 2 - inverted LED pyramid in the I/O port grid, 01 = LED on
; the pattern is the 4x8 table at 2000H (a framebuffer); the code only
; scans it out cell by cell, one row per second
; (OUT's port is fixed per instruction on the 8085, and this simulator
; executes its assembled listing so self-modifying OUT does not take -
; hence one OUT per grid cell)

# ORG 7000H

LXI SP,0FFFFH
LXI H,2000H

MOV A,M
OUT 00H
INX H
MOV A,M
OUT 01H
INX H
MOV A,M
OUT 02H
INX H
MOV A,M
OUT 03H
INX H
MOV A,M
OUT 04H
INX H
MOV A,M
OUT 05H
INX H
MOV A,M
OUT 06H
INX H
MOV A,M
OUT 07H
INX H
CALL DELAY

MOV A,M
OUT 10H
INX H
MOV A,M
OUT 11H
INX H
MOV A,M
OUT 12H
INX H
MOV A,M
OUT 13H
INX H
MOV A,M
OUT 14H
INX H
MOV A,M
OUT 15H
INX H
MOV A,M
OUT 16H
INX H
MOV A,M
OUT 17H
INX H
CALL DELAY

MOV A,M
OUT 20H
INX H
MOV A,M
OUT 21H
INX H
MOV A,M
OUT 22H
INX H
MOV A,M
OUT 23H
INX H
MOV A,M
OUT 24H
INX H
MOV A,M
OUT 25H
INX H
MOV A,M
OUT 26H
INX H
MOV A,M
OUT 27H
INX H
CALL DELAY

MOV A,M
OUT 30H
INX H
MOV A,M
OUT 31H
INX H
MOV A,M
OUT 32H
INX H
MOV A,M
OUT 33H
INX H
MOV A,M
OUT 34H
INX H
MOV A,M
OUT 35H
INX H
MOV A,M
OUT 36H
INX H
MOV A,M
OUT 37H
RST 1

; 1 s at 1 MHz would be LXI B,0A2C3H - short count for the simulator
DELAY: LXI B,0010H
DLOOP: DCX B
MOV A,B
ORA C
JNZ DLOOP
RET

; the picture itself - edit these four lines to change the pattern
# ORG 2000H
# DB 01H,01H,01H,01H,01H,01H,01H,00H
# DB 00H,01H,01H,01H,01H,01H,00H,00H
# DB 00H,00H,01H,01H,01H,00H,00H,00H
# DB 00H,00H,00H,01H,00H,00H,00H,00H
