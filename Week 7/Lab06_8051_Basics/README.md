# Lab 06 — 8051 Basics (17/09/2026)

Handout: `MPMC_Lab_06.pdf`. First lab of the **8051** half — new chip, new
assembly dialect, new simulator. No ports, no interrupts: everything
happens in the 8051's **internal RAM**, and that is where results are read.

The `.asm` files are in **Intel syntax** (`ORG 0000H … END`), which is what
the lab's MCU 8051 IDE, Keil and EdSim51 all accept.

| Q | File | Operands | Expect in internal RAM |
|---|---|---|---|
| 1 | `q1_arithmetic.asm` | `30H=5A` (90), `31H=07` | `40H=61` sum · `41H=53` difference · `42H=76`,`43H=02` product `0276H`=630 · `44H=0C` quotient · `45H=06` remainder |
| 2 | `q2_logical.asm` | `30H=5A`, `31H=0F` | `50H=0A` AND · `51H=5F` OR · `52H=55` XOR · `53H=A5` complement of `5A` |
| 3 | `q3_block_move.asm` | `40H–44H = 11 22 33 44 55` | `60H–64H = 11 22 33 44 55` |

All three were **run and verified** on this Mac with `tools/run8051.sh`
(sdas8051 + the s51 simulator that ships with SDCC):

```bash
tools/run8051.sh "Week 7/Lab06_8051_Basics/q1_arithmetic.asm" 200 0x30 0x67
```

It prints the register state and an internal-RAM dump. Bytes the program
never wrote show random values — s51 deliberately randomises
uninitialised RAM; only the addresses in the table above mean anything.

## Running in the lab (MCU 8051 IDE)

1. *Project → New*, processor `AT89C51`. New file, paste, save as `.asm`,
   add to project.
2. **F2** (Start simulator) assembles it. Errors show in *Messages*.
3. **F7** steps one instruction; *Run* goes until you stop it. Each program
   ends in `HERE: SJMP HERE`, so Run simply parks there.
4. Read results in the **internal RAM grid** (bottom-left hex table): row
   `40` for Q1, row `50` for Q2, rows `40` and `60` for Q3. Registers
   (A, B, R0–R7, PSW) are beside it.

## What is different from the 8085

| 8085 | 8051 | Note |
|---|---|---|
| `MVI A,5AH` | `MOV A,#5AH` | `#` marks an immediate. Without it, `5AH` is an **address** |
| `LDA 2050H` / `STA 2050H` | `MOV A,30H` / `MOV 30H,A` | internal RAM is addressed directly by any `MOV`, even `MOV 40H,#11H` |
| `SUB B` | `CLR C` then `SUBB A,31H` | the 8051 has **only** subtract-with-borrow, so clear CY first |
| repeated addition | `MUL AB` | one instruction; low byte in A, high byte in **B** |
| repeated subtraction | `DIV AB` | quotient in A, remainder in **B**; divide by zero sets OV |
| `MOV A,M` via HL | `MOV A,@R0` | only **R0 and R1** can be pointers, and only into internal RAM |
| `DCR C` + `JNZ` | `DJNZ R2,LOOP` | decrement-and-jump in one instruction |
| `ANA` `ORA` `XRA` `CMA` | `ANL` `ORL` `XRL` `CPL A` | same operations, new names |
| `RST 1` to stop | `HERE: SJMP HERE` | a microcontroller never "ends"; it parks in a loop |

Two traps worth knowing for the viva:

- **`MOV @R1,@R0` does not exist.** Memory-to-memory moves go through A,
  which is why the block-move loop is `MOV A,@R0 / MOV @R1,A`.
- **`MOV A,30H` vs `MOV A,#30H`.** The first loads the *contents of* RAM
  location 30H, the second loads the *number* 30H. Dropping the `#` is
  the single most common 8051 bug.
