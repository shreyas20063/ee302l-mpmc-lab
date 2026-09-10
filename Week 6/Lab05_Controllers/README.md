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

1. Assemble, `Start From 7000`, Normal speed. **`R7.5` must read 0 before Run.**
2. Run. Devices tab: port `82H` (row 80, column 2) = `00`, gate open.
3. A train arrives: set port `80H` (row 80, column 0) to `01`, then click
   `R7.5` on. Port 82H → `01` (gate closed), `9000H` → `01` in the Memory tab.
4. After the 1 s delay the ISR polls the sensor. Train passes: set port
   `80H` back to `00`. Port 82H → `00` (gate open).
5. Click `R7.5` off — the ISR returns to the main loop.
6. Next train: repeat 3–5; `9000H` → `02`, and so on.

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

**Task 1: how `RET` is made honest in this simulator.** Last week we found
this simulator does not push a return address on RST 7.5, so an ISR's
`RET` pops whatever is on the stack. The handout demands `RET`. Solution:
the main program is `MAIN: CALL MONITOR / JMP MAIN` with
`MONITOR: JMP MONITOR`. The call parks a return address on the stack and
the infinite monitoring loop lives *inside* the subroutine, so the ISR's
`RET` always finds that address, lands on `JMP MAIN`, and the stack is
re-primed by the next `CALL`. On real hardware this is simply correct
code — `RET` returns to the interrupted spot. The cost: if `R7.5` is
already 1 when `EI` runs, the interrupt fires before the first `CALL` and
the stack is empty — hence the "must read 0 before Run" rule.

**Task 1: the ISR tail.** `SIM 18H` (reset the 7.5 flip-flop — textbook),
`SIM 08H` (release the simulator's sticky reset bit), then
`RIM / ANI 40H / JNZ` waits for the toggle to be released so the ISR is
not re-entered the instant `EI` runs. On hardware the wait exits at once.

**Task 1: gate logic.** `IN 80H / ORA A / JNZ WAIT` keeps the gate closed
while the sensor reads anything but `00`. Count increments *before*
closing, so the count reflects arrivals even if a train sits there.

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
