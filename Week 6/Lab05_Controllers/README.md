# Lab 05 — Interrupt-Driven Controllers (10/09/2026)

Handout: `MPMC Lab 05.pdf` (this folder). Two tasks, both built on the
RST 7.5 / stack machinery from Week 5.

**Both tasks: code at `7000H` — `Start From = 7000`.** Task 2 uses `2000H`
and `2001H` as memory-mapped outputs, so code cannot live at 2000H this
week. Stack at `2700H`.

| Task | File | Ports / memory |
|---|---|---|
| 1 | `task1_railway_crossing.asm` | in `80H` sensor, out `82H` gate, count `9000H`, ISR at `003CH` |
| 2 | `task2_tank_level.asm` | readings `9000H–9009H`, current pump `2000H`, pump table `2002H–200BH`, alarm `2001H`, counts `3000H–3003H`, ISR via `RST 7` at `0038H` |

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

## Task 2 — the model, as clarified by the TAs

Ten tanks, each with its own pump, all joined by an overflow pipe at the
top. The loop visits one tank per iteration:

- Each tank's pump state lives in a table, `2002H–200BH` (tank 1 at
  `2002H`). `2000H` mirrors **the pump of the tank being processed**:
  LOW → on, HIGH → off, NORMAL → that tank's pump keeps its own state.
- After tank *k*, the **running average of the first k tanks** is computed
  (stored at `3003H`). If it is above 80% of full scale, the overflow
  interrupt fires immediately — **every pump off** (the whole table is
  cleared), alarm `FF` for a second, `00` — and monitoring continues with
  tank *k+1*.
- After tank 10 the cycle restarts.

## Task 2 — running it

Assemble, `Start From 7000`, Run. It cycles forever, so Stop to read:

| Address | Expect (shipped data) |
|---|---|
| `3000H` low count | `03` |
| `3001H` normal count | `05` |
| `3002H` high count | `02` |
| `3003H` running average (after tank 10) | `05` |
| `2000H` pump of tank 10 | `00` (tank 10 is NORMAL, so its own pump stays off) |
| `2002H–200BH` pump table | `01 00 00 01 00 00 00 00 01 00` — on for the three LOW tanks (1, 4, 9) |
| `2001H` alarm | `00` |

With the shipped data the running average never crosses 80%. To see the
overflow path, load this instead — it alarms after tanks 1 and 2, then
settles:

```
# ORG 9000H
# DB 0AH,0AH,03H,04H,05H
# ORG 9005H
# DB 06H,07H,08H,09H,0AH
```

Running sums 10, 20, 23, 27, 32, 38, 45, 53, 62, 72 against the 80% line
8·k = 8, 16, 24 … 80 → above it only at k = 1 and 2.

**To watch an alarm happen:** put a breakpoint (`#`) on the `RST 7` line.
Each time it pauses there, `3003H` holds the average that tripped it and
`2000H` the pump about to be forced off; Continue, then Stop a moment
later to catch `2001H = FF` (or watch it in the Memory tab at Normal
speed).

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

**Task 2: the 80% test without fractions.** After *k* tanks, average >
80% of `0AH` means average > 8, i.e. sum > 8·k. The program computes
8·k as `k` rotated left three times (`RLC` ×3 — a multiply by 8 that never
overflows since 8·10 = 80) and compares it with the running sum, so the
decision keeps the fractional part: an average of 8.5 correctly alarms.
The integer running average is also computed (repeated subtraction of
*k*) and stored at `3003H` because the handout asks for it.

**Task 2: per-tank pump memory.** Ten pumps need ten states, so the
controller keeps a table (`2002H–200BH`) and `2000H` shows the entry for
the tank in hand. LOW writes on, HIGH writes off, NORMAL reads the entry
back unchanged — hysteresis per tank, not carried across neighbours.
"Reset to zero" (`ALLOFF`) runs once at start, and again inside the ISR,
because an overflowing group must stop *all* filling, not just the tank
that happened to tip the average.

**Task 2: register budget.** The three counts live in memory
(`3000H–3002H`, updated with `LDA / INR / STA`) rather than in registers,
which frees B and C for the per-iteration division. The loop ends by
checking the pointer: `MOV A,L / CPI 0AH` — the low byte of HL walks `00`
to `0A`, and `L + 1` doubles as the tank count *k*. The ISR pushes the D
pair because `DELAY` uses DE and E is the running sum.

**Task 2: why the pump table is addressed through HL, not `STAX B`.**
The simulator's `STAX`/`LDAX` are broken (open issue #1 on its GitHub:
the address is computed as `B << (8+C)` because of a Java precedence
bug), and for `C ≥ 05` that indexes past the 64 K array and **kills the
run thread silently** — registers freeze while the Stop button stays up.
So each branch saves HL, points it at `2002H + tank` (`MVI H,20H / INR L /
INR L`), reads or writes with `MOV`, and restores HL. `MOV M,r` is also
used in `ALLOFF` instead of `MVI M`, staying within instructions proven
to work here.

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
- `STAX B`/`STAX D`/`LDAX` are miscomputed (`B<<8+C` precedence bug,
  issue #1) — avoid them; an out-of-range index kills the run thread with
  no error shown.
