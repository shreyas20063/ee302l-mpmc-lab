;=============================================================================
; EE302L - Week 1 : Addition of two 8-bit numbers, WITH carry propagation
; Target : Intel 8085 (Jubin Mitra 8085 Simulator v2)
;
; add_8bit.asm keeps only the low 8 bits. 88H + 88H = 110H stores 10H there
; and silently loses the overflow. This version gives a full 16-bit result.
;
; Memory map
;   7501H : first  operand   (input)
;   7502H : second operand   (input)
;   7503H : sum, low  byte   (output)
;   7504H : sum, high byte   (output) - 00H or 01H
;=============================================================================

# ORG 7000H

LXI H,7501	; HL points to 7501H, address of 1st number
MVI C,00	; C accumulates the high byte of the result
MOV A,M		; Move 1st number into accumulator
INX H		; HL points to 7502H
ADD M		; A = 1st + 2nd, CY set on 8-bit overflow

;-----------------------------------------------------------------------------
; >>> YOUR PART <<<
;
; A now holds the low byte, and CY tells you whether it overflowed. You saw
; CY sitting at 0 in the Registers panel on the last run - this is the case
; where it goes to 1. Capture that carry into register C, then fall through
; to the stores below.
;
; Three correct approaches, and they differ in ways worth thinking about:
;
;   (a) Branch on it  - JNC over an INR C. Reads like the logic you would say
;                       out loud. Costs a label, and the two paths take a
;                       different number of T-states - which you can now read
;                       straight off the Assembler tab and compare.
;
;   (b) Add it in     - ACI 00H adds the carry arithmetically into a zeroed
;                       accumulator. Branch free and constant time, but it
;                       clobbers A, so park the low byte somewhere first.
;
;   (c) Rotate it in  - RAL shifts CY into bit 0 of a register. Compact, but
;                       it moves the other 7 bits too, so the register has to
;                       be set up correctly first.
;
; Roughly 3-5 lines. Option (a) is the conventional lab answer, (b) is the
; one worth defending if a TA asks why.
;
; TODO: capture the carry into C
;-----------------------------------------------------------------------------

INX H		; HL points to 7503H
MOV M,A		; Store low byte
INX H		; HL points to 7504H
MOV M,C		; Store high byte
RST 1		; Terminate, vectors to 0008H

;-----------------------------------------------------------------------------
; Test data. 88H + 88H = 110H, so a correct run gives
;   7503H = 10H  and  7504H = 01H
; Swap back to 12H,13H to check the no-carry path still gives 25H / 00H.
;-----------------------------------------------------------------------------

# ORG 7501H
# DB 88H, 88H
