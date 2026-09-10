# Lab 05 — Interrupt-Driven Controllers (10/09/2026)

Handout: `MPMC Lab 05.pdf` (this folder). Two tasks, both built on the
RST 7.5 / stack machinery from Week 5.

**Both tasks: code at `7000H` — `Start From = 7000`.** Task 2 uses `2000H`
and `2001H` as memory-mapped outputs, so code cannot live at 2000H this
week. Stack at `2700H`.

| Task | File | Ports / memory |
|---|---|---|
| 1 | `task1_railway_crossing.asm` | in `80H` sensor, out `82H` gate, count `9000H`, ISR at `003CH` |
| 2 | `task2_tank_level.asm` | readings `9000H–9009H`, pump `2000H`, alarm `2001H`, counts `3000H–3003H`, ISR via `RST 7` at `0038H` |

## Task 1 — running it

The I/O grid only accepts edits while the CPU is stopped, so the sensor
is set **before** Run and the ISR has a poll limit for when it cannot be
cleared live.

1. Optional: type `01` into port `80H` (row 80, column 0) while stopped —
   a train is sitting at the sensor. Leave it `00` for the quick case.
2. Assemble, `Start From 7000`, Normal speed. (`R7.5` left at 1 from a
   previous run is now harmless — the program disables interrupts first
   and only enables them once a return address is on the stack.)
3. Run. Port `82H` (row 80, column 2) = `00`, gate open.
4. Click `R7.5` on. Port 82H → `01` (gate closed), `9000H` → `01`.
   - sensor `00`: gate reopens after the 1 s delay
   - sensor `01`: gate stays closed through ten more polls, then reopens
5. Click `R7.5` off — the ISR returns to the main loop.
6. Next train: click `R7.5` on again; `9000H` → `02`, and so on. To change
   the sensor, Stop first, edit port 80H, Run again.

## Task 2 — running it

Assemble, `Start From 7000`, Run. It cycles forever, so Stop to read:

| Address | Expect (shipped data) |
|---|---|
| `3000H` low count | `03` |
| `3001H` normal count | `05` |
| `3002H` high count | `02` |
| `3003H` average | `05` |
| `2000H` pump | `01` (last sample `06` was NORMAL, so the ON from `00` before it stays) |
| `2001H` alarm | `00` |

To see the overflow path, load a high data set and rerun:
`# DB 09H,0AH,0AH,09H,08H` / `# DB 0AH,09H,0AH,09H,0AH` → sum 94, average
`09`, counts `00/01/09`, pump ends `00`, and every cycle the alarm goes
`FF` for a second then `00`. (Stop mid-cycle to catch `2001H = FF`, or
watch it in the Memory tab at Normal speed.)

## Design notes — the parts a TA will ask about

**Task 1: how `RET` is made honest in this simulator.** This simulator
does not push a return address on RST 7.5 (verified in its source — see
below), so an ISR's `RET` pops whatever is on the stack. The handout
demands `RET`. Solution: `MAIN: CALL MONITOR / JMP MAIN`, with the
infinite monitoring loop *inside* `MONITOR`. The call parks a return
address; `EI` is issued only inside `MONITOR`, after that address exists;
so whenever the ISR's `RET` runs it finds a valid address, lands on
`JMP MAIN`, and the next `CALL` re-primes the stack. `DI` is the very
first instruction so state left over from an earlier run (the simulator
does not reset the CPU between runs) cannot fire an interrupt early. On
real hardware all of this is simply correct, ordinary code.

**Task 1: the ISR tail.** `SIM 18H` (reset the 7.5 flip-flop — textbook),
`SIM 08H` (release the simulator's sticky reset bit), then
`RIM / ANI 40H / JNZ` waits for the toggle to be released so the ISR is
not re-entered the instant `EI` runs. On hardware the wait exits at once.

**Task 1: gate logic.** The ISR polls `IN 80H` continuously as required
and opens the gate the moment it reads `00`. It also caps the polling at
ten rounds (`MVI B,0AH / DCR B / JNZ`): on real hardware that is stuck-
sensor recovery; in the simulator it is what lets the demo finish, since
port cells cannot be edited during a free run. Count increments *before*
closing, so it reflects arrivals even if a train sits there.

**Task 2: "generate an interrupt" from software.** The overflow condition
is computed by the program, so no external line can raise it. The 8085's
own mechanism for that is the `RST n` instruction — a one-byte software
interrupt that pushes the PC and jumps to a vector. `RST 7` → `0038H`,
where a `JMP ALARM` sits (vectors are 8 bytes apart; a jump keeps the
handler out of the table). Because `RST` *does* push in this simulator,
the handler's `RET` is a genuine return — no tricks needed.

**Task 2: threshold without division.** Average > 80% of full scale
(`0AH`) means sum > 80, i.e. sum ≥ 81 → `CPI 51H / JC START`. The
integer average is still computed (repeated subtraction of 10) and stored
at `3003H` because the handout asks for it. Using the sum for the
decision keeps the fractional part: an average of 8.5 correctly alarms.

**Task 2: pump hysteresis.** LOW turns it on, HIGH turns it off, NORMAL
leaves it alone — so the pump's final state depends on sample order, and
"reset to zero" happens once at program start, not every cycle.

**Task 2: loop termination without a counter register.** B, C, D, E are
all in use (three counts and the sum), so the loop ends by checking the
pointer: `MOV A,L / CPI 0AH` — the low byte of HL walks `00` to `0A`.

## New instructions this week

| Instruction | Meaning |
|---|---|
| `IN 80H` | read input port 80H into A (the value typed into that cell of the I/O grid) |
| `RST 7` | software interrupt: push PC, jump to `0038H` |
| `SUI 0AH` | subtract immediate |
| `RIM` | read interrupt status; bit 6 = RST 7.5 line pending |

## How this simulator really handles interrupts (from its source, `Matrix.java` / `Assembler.java`)

Read from <https://github.com/8085simulator/8085simulator>, 10/09/2026.

- `interruptProcess()` runs after **every** instruction. For RST 7.5 it
  is literally `if(IE==1 && R75==1 && RR75==0 && !(M75==1 && MSE==1))
  { IE=0; PC=0x003C; }` — **no push of PC**. `CALL` and `RST n` do push.
- `R75` is the Registers-panel cell, toggled only by clicking it; the CPU
  never clears it. It is a **level**. `RIM` bit 6 returns it directly.
- `SIM` stores bit 4 into `RR75`, which stays until the next `SIM`. While
  it is 1, RST 7.5 is blocked. Masks are stored regardless of MSE, but
  only block when both `M75` and `MSE` are 1.
- `EI` sets `IE=1` immediately, and `interruptProcess()` runs right after
  it — so with `R75` still 1 the ISR re-enters *before* `RET` executes.
  Hence the `RIM / ANI 40H / JNZ` wait for the cell to be toggled off.
- Run does **not** reset the CPU: `IE`, `RR75`, `R75`, registers and
  memory persist between runs. *Settings → Clear Memory* makes a fresh
  CPU.
- Repaint: at Normal speed the Registers panel refreshes every
  instruction, but Memory and I/O tables only when the clock-cycle
  counter is an **exact** multiple of 10,000 — which is why the port grid
  freezes for stretches and catches up on Stop. Ultimate refreshes only on
  halt; Step-by-step refreshes every step.
- **Breakpoint:** click the `√` in the `*` column of a row in the
  Assembler workspace so it becomes `#`. The run pauses there with
  Continue/Forward/Backward available, and the I/O grid is editable.
- `DAA` tests the high nibble of the *original* A and XORs the new carry
  with the old one: `99H+01H` → `A0H`; wrap BCD counters explicitly.
- The default "stop at mnemonic" is `RST 1`.
