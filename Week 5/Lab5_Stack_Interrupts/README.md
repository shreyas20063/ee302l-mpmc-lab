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

1. Assemble, `Start From 2000`, simulation speed **Normal**.
2. **Before pressing Run, make sure the `R7.5` cell in the Registers panel
   reads 0.** If it is already 1 when `EI` executes, the simulator jumps
   to the ISR with nothing on the stack and the run is lost — Stop, set it
   to 0, Run again.
3. Run. Devices tab: port `01H` (row 00, column 1) counts `00 01 … 99 00`.
4. Click `R7.5` (it goes to 1): port `02H` blinks `FF/00` five times while
   port 01H freezes.
5. Click `R7.5` again (back to 0): the ISR finishes, port 01H resumes from
   the frozen value. Leave it at 1 and the count simply stays frozen with
   the alarm off until you release it.
6. Repeat 4–5 as often as you like — every click is serviced.
7. It never halts by design — Stop when done.

### What this simulator actually does on RST 7.5 (found by decompiling the jar)

- **It does not push a return address.** `interruptProcess()` just sets
  `IE = 0` and `PC = 003CH`. Only CALL/RST opcodes push. So an ISR's `RET`
  pops whatever happens to be on top of the stack. This is why a textbook
  ISR ending in `EI / RET` re-entered forever: `RET` popped `0000`, the
  finger walked through NOPs from `0000H` straight back into `003CH`.
  Design response: the main loop is `MAIN: CALL TICK / JMP MAIN`, all work
  inside subroutines, so the stack top is *always* a valid return address.
  On real hardware `RET` returns to the interrupted instruction; here it
  returns from the innermost subroutine and merely shortens one tick.
- **The `R7.5` cell is a level, not a latch**, never cleared on
  acknowledge, and `RIM` bit 6 reports it directly. `SIM` bit 4 sets a
  sticky reset that blocks RST 7.5 until another `SIM` clears it. Hence the
  ISR tail: `SIM 18H` (textbook flip-flop reset), `SIM 08H` (release the
  simulator's sticky bit), then `RIM / ANI 40H / JNZ` to wait for the
  toggle to be released, then pops, `EI`, `RET`.
- **Its `DAA` checks the high nibble of the *original* A**, so `99H + 01H`
  gives `A0H`, not `00H`. The 99 → 00 wrap is done explicitly with
  `CPI 99H`; `DAA` handles the other 99 increments (CY is always 0 before
  it, so the documented CY bug never triggers).
- Normal speed repaints every 10,000 T-states; a too-small delay aliases
  against that and the display appears to skip values (the "only odd
  numbers" symptom). `1000H` iterations ≈ 100k T-states per count.

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
  handout's byte); `EI` turns interrupts on globally. Both are needed.
- Counting lives in `TICK`: show B, wait, `ADI 01H / DAA` to stay decimal,
  with the 99 → 00 wrap done by `CPI 99H` (see simulator note above).
- The ISR sits at `003CH` because that is where the CPU jumps on RST 7.5.
- `PUSH PSW / PUSH B / PUSH D` on entry, mirrored `POP`s on exit: the ISR
  saves everything it touches. "Save the count on the stack" is `PUSH B`.
- `EI` before `RET`: the CPU disables interrupts when it accepts one.
- Deviations from the handout example, all forced by this simulator:
  `SIM` before `EI`; main loop calls a subroutine instead of `JMP MAIN`;
  ISR pushes PSW/B/D not just B; ISR resets and releases the 7.5 flip-flop
  and polls `RIM` before returning.

## New instructions this week

| Instruction | Meaning |
|---|---|
| `PUSH B` | SP −= 2, write B and C onto the stack |
| `POP B` | read B and C back, SP += 2 |
| `PUSH PSW` / `POP PSW` | same for A + flags |
| `EI` | enable interrupts |
| `SIM` | write A into the interrupt mask register (bit 3 = mask-set-enable, bits 0–2 = masks for 5.5/6.5/7.5, 0 = unmasked, bit 4 = reset the 7.5 flip-flop) |
| `RIM` | read interrupt status into A (bit 6 = RST 7.5 pending, bit 3 = IE) |
| `CPI 99H` | compare A with a constant |
| `DAA` | decimal-adjust A after a BCD add |
| `ADI 01H` | add immediate |
