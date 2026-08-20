# Lab 3 — Loops and Branching (20/08/2026)

Handout: `Lab_3_Experiments.pdf` (this folder). Data moved to `2500H` this
week; code still assembles at `7000H` — **Start From = 7000** as always.

| Exp | File | Data | Expected |
|---|---|---|---|
| 1 | `exp1_production_mult.asm` | `40H × 05H` | `2502H=40H`, `2503H=01H` (320) |
| 2 | `exp2_fuel_div.asm` | `EAH ÷ 0CH` (234/12) | `2502H=13H` (19), `2503H=06H` |
| 3 | `exp3_limit_check.asm` | `50H` in `[30H,70H]` | `2503H=00H` |

## Edge cases each program survives

- **Exp 1, batches = 0** — guarded with `ORA A / JZ` before the loop.
  Without it, `DCR C` on `00H` wraps to `FFH` and the loop runs 256 times.
  Result also genuinely needs 16 bits: `FFH × FFH = FE01H` fits, so no
  overflow case exists.
- **Exp 2, fuel = 0** — division by zero would loop forever (subtracting 0
  never borrows). Detected up front; stores `FFH` quotient + distance as
  remainder, documented convention. Also fine when distance < fuel:
  quotient 0, remainder = distance.
- **Exp 3, boundaries** — reading equal to min or max is NORMAL per the
  spec (`≤`). `CMP` leaves Z=1, CY=0 on equality: the `JC LOW` falls
  through at reading=min, and the explicit `JZ OK` catches reading=max.

## Test matrix for the bench

| Exp | Change `2500H`-area to | Expect |
|---|---|---|
| 1 | batches (`2501H`) = `00H` | result `0000H` |
| 2 | fuel (`2501H`) = `00H` | `2502H=FFH`, `2503H=` distance |
| 2 | distance < fuel (e.g. `05H`, `0CH`) | quotient 0, remainder 05H |
| 3 | `20H` / `90H` / `30H` / `70H` | `01` / `02` / `00` / `00` |

Change values fast: Memory tab → "Store directly to specified memory
location" — no reassembly needed.

## New instructions this week

| Instruction | Meaning |
|---|---|
| `DAD D` | HL = HL + DE, 16-bit add in one go. Only touches CY. The "repeated addition" happens 16-bit wide so the running total never overflows an 8-bit register |
| `DCR C` | C = C − 1. Sets Z when it hits zero — the loop's countdown. (Sibling of `INR`; like `INR` it leaves CY alone) |
| `JNZ` / `JZ` | jump if Z clear / set — the loop-back and the zero-guard |
| `JC` | jump if CY set — after `CMP`, that means "A was smaller" |
| `JMP` | unconditional jump |
| `CMP B` | compute A − B, throw away the result, keep the flags. CY=1 → A < B, Z=1 → equal. Comparison IS subtraction |
| `ORA A` | A OR A — A unchanged, but flags now describe A. The idiomatic "is A zero?" test |
| `SHLD 2502H` | store L at `2502H`, H at `2503H` — 16-bit result lands LSB-first, exactly the layout the handout demands |
