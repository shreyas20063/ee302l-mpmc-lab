# Lab — Stack and Interrupts (03/09/2026)

Handout: `MPMC_Lab_TA1-1.docx` (PDF copy alongside). Three questions plus a
worked RST 7.5 example in the handout itself.

**Code starts at `2000H` this week — `Start From = 2000`**, not 7000. Q3
mandates it, and the other two follow for consistency. Stack at `2700H`.

| Q | File | Expect |
|---|---|---|
| 1 | `q1_reverse_stack.asm` | `2050H–2054H`: `11 22 33 44 55` → `55 44 33 22 11` |
| 2 | `q2_add_subroutine.asm` | `2060H = 0F`; after the run **B = 11, C = 22** (survived the call) |
| 3 | `q3_bcd_counter_isr.asm` | port 01H counts `00 01 … 09 10 … 99 00`; fire RST 7.5 → port 02H blinks `FF/00` ×5, then port 01H resumes from the same count |

## Running Q3 — firing the interrupt

1. Set simulation speed to **Normal**, `Start From 2000`, Run.
2. Watch port `01H` count in the Devices tab (row 00, column 1).
3. **Registers tab → the teal interrupt row → click `R7.5` on, then
   straight back off** (pulse it like a button). That is the emergency
   event. Port `02H` (row 00, column 2) blinks five times while port 01H
   freezes; then counting continues from the paused value.

   Simulator finding (03/09): accepting the interrupt does **not** clear
   the RST 7.5 latch here, so the ISR re-fired forever after its `EI`.
   Fix in the ISR: `MVI A,18H / SIM` — bit 4 of the SIM byte resets the
   7.5 flip-flop. Real hardware clears it on acknowledge, but resetting
   it explicitly is standard practice anyway. It sits *before* `POP PSW`
   so the restored A is not clobbered.
4. It never halts by design — Stop when done.

## Why each program is shaped the way it is

**Q1 — reversal by LIFO.** The stack is last-in-first-out. Push five
bytes in order, pop five bytes back into the same slots, and the order
inverts with no comparisons or swaps. `PUSH` only takes register *pairs*,
so the byte rides in B and C is pushed along as junk. The loop counter
lives in D, **not** C — C is inside the pushed pair, and `POP B` would
overwrite it with the stale pushed value every lap (infinite loop).

**Q2 — why preserve BC.** The subroutine uses B and C as scratch. Without
`PUSH B`/`POP B` around the call, main's B and C come back as `05`/`0A`.
Bench demo: comment the two lines out, rerun, watch B and C get wrecked.
That is the entire point of the question in one experiment.

**Q3 — the interrupt round trip.**
- `MVI A,08H / SIM` sets the mask-enable bit and unmasks RST 7.5 (the
  handout's byte). `EI` turns interrupts on globally. Both are needed.
- `ADI 01H / DAA` keeps the count decimal. `DAA` fixes `09+1 = 0AH` to
  `10H`, and `99+1 = 9AH` to `00H`. (The simulator's known DAA carry bug
  only affects cases where CY is already set before `DAA`; `ADI 01H` can
  never carry, so it doesn't bite here.)
- The ISR sits at `003CH` because that is where the CPU jumps on RST 7.5
  — a fixed vector, same idea as `RST 1` → `0008H`.
- `PUSH PSW / PUSH B / PUSH D` on entry, mirrored `POP`s on exit: the
  ISR saves everything it touches. B is the count — the "save the count
  on the stack" requirement is literally `PUSH B`.
- `EI` before `RET`: the CPU disables interrupts when it accepts one.
  Forget this and the second emergency is ignored forever.

## New instructions this week

| Instruction | Meaning |
|---|---|
| `PUSH B` | SP −= 2, write B and C onto the stack |
| `POP B` | read B and C back, SP += 2 |
| `PUSH PSW` / `POP PSW` | same for A + flags |
| `EI` | enable interrupts |
| `SIM` | write A into the interrupt mask register (bit 3 = mask-set-enable, bits 0–2 = masks for 5.5/6.5/7.5, 0 = unmasked) |
| `DAA` | decimal-adjust A after a BCD add |
| `ADI 01H` | add immediate |
