;=============================================================================
; EE302L - Week 1 : Addition of two 8-bit numbers
; Target : Intel 8085 (Jubin Mitra 8085 Simulator v2)
;
; VERIFIED: assembles to 9 bytes at 7000H and runs correctly.
;           12H + 13H -> 7503H = 25H, CY = 0
;
; Memory map
;   7501H : first  operand   (input)
;   7502H : second operand   (input)
;   7503H : sum, low byte    (output)
;
; HL is used as a moving pointer. MOV A,M and ADD M both read through HL,
; so a single INX H between them walks from one operand to the next.
;
; NOTE: this version stores only the 8-bit sum. If the operands total more
;       than FFH the carry is discarded - see add_8bit_carry.asm
;=============================================================================

# ORG 7000H

LXI H,7501	; HL points to 7501H, address of 1st number
MOV A,M		; Move 1st number into accumulator
INX H		; HL points to 7502H
ADD M		; Add the 2nd number, CY set on 8-bit overflow
INX H		; HL points to 7503H
MOV M,A		; Store result in 7503H
RST 1		; Terminate, vectors to 0008H

;-----------------------------------------------------------------------------
; Input data. These load into the Memory Editor, not the Assembler Workspace.
;-----------------------------------------------------------------------------

# ORG 7501H
# DB 12H, 13H
