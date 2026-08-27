# Lab — Memory Access, Output Ports, Delay Generation (27/08/2026)

Handout: `MPMC_Lab3_Questions.pdf` (this folder). It titles itself "Lab 3"
even though last week's file did too — week folders are authoritative here.

Code at `7000H` — **Start From = 7000** as always.

| Task | File | Expect |
|---|---|---|
| 1 | `task1_adc_filter.asm` | `3000H-3004H` = `85 80 FF 90 A5`, `4000H` = `05` |
| 2 | `task2_led_pyramid.asm` | Devices tab: pyramid of `01`s across grid rows 00/10/20/30 (see below), rows appearing 1 s apart |
| 3 | `task3_traffic_light.asm` | port 20H cycles `04` (5 s) → `02` (2 s) → `01` (5 s), two full cycles, then halts with `01` showing |

## Where to look

Ports are NOT in the Memory tab — **Devices tab → I/O Port Editor**. `OUT`
writes land in that grid (row = high nibble of the port number).

Task 3 runs the sequence twice and halts (cycle count kept at `2100H`,
set by the `MVI A,02H` near the top). A real controller loops forever —
say so in the record; the `JNZ START` becoming `JMP START` is the
one-line difference, worth mentioning if asked.

## The 1-second delay, shown work

At 1 MHz, 1 second = 1,000,000 T-states. The delay loop body:

| Instruction | T-states |
|---|---|
| `DCX B` | 6 |
| `MOV A,B` | 4 |
| `ORA C` | 4 |
| `JNZ` (taken) | 10 |
| total per lap | **24** |

Iterations = 1,000,000 / 24 ≈ 41,667 = `A2C3H`. Call/setup overhead is
~40 T-states — error well under 0.01%.

**Verify it without a wall clock:** note the Clock Cycle Counter, run one
`CALL DELAY`, check it advanced by ≈ 1,000,040. The simulator is not
cycle-accurate in real time, so the counter — not a stopwatch — is the
evidence the delay is right.

**The files ship with `0010H` as the delay count** so simulator runs
finish instantly and the I/O grid actually paints (the simulator only
repaints on halt, and it runs far slower than a real 1 MHz chip). For
the record and viva, quote `A2C3H` and the arithmetic above — that is
the answer for real hardware at 1 MHz; the short count is a simulator
convenience, not the design value.

**Where the pattern shows:** Devices tab → I/O Port Editor. Task 2 draws
one `01` per lit LED — the port number picks the cell (high nibble =
grid row, low nibble = column):

```
row 00:  01 01 01 01 01 01 01 00
row 10:  00 01 01 01 01 01 00 00
row 20:  00 00 01 01 01 00 00 00
row 30:  00 00 00 01 00 00 00 00
```

The alternate `task2_led_pyramid_alt_byteports.asm` reads the circuit as
"8 LEDs share one port" and outputs `FE 7C 38 10` to ports 01–04 instead.
Task 3's port 20H is the row labelled `20`, first column.

## Edge cases and details that earn marks

- Task 1: sample exactly `80H` **is** a crossing (spec says ≥). `CPI 80H`
  leaves CY=0 when A=80H, so `JC SKIP` correctly falls through. The data
  includes the `7FH`/`80H` boundary pair to prove it.
- Task 2: "keep previous rows illuminated" costs nothing — output ports
  are latches; they hold their byte until the next `OUT` to the same port.
- Task 2/3: `DCX` (unlike `DCR`) sets **no flags** — that is why the pair
  is tested with `MOV A,B / ORA C` instead of a bare `JNZ`.
- Task 3: the 1-second routine uses **DE** so it cannot disturb B
  (seconds) or C (signal counter) — register allocation as a correctness
  tool, same idea as `INR C` not touching A in Lab 3's division.
- Both delay users set `LXI SP,0FFFFH` first: `CALL` pushes a return
  address, and the reset-default SP is not somewhere you want writes.

## New instructions this week

| Instruction | Meaning |
|---|---|
| `CPI 80H` | compare A with a constant: invisible `A − 80H`, notes only. CY=1 → A was smaller |
| `STAX D` | store A at the address in DE — `STA`'s pointer cousin, and the mirror of `LDAX` |
| `OUT 01H` | send A to output port 01H. Ports are a separate 256-slot address space reached only by `IN`/`OUT` |
| `CALL DELAY` | jump to a label AND push the return address on the stack |
| `RET` | pop that address, jump back — `CALL`'s round trip home |
| `LXI SP,0FFFFH` | aim the stack pointer before the first `CALL` |
| `DCX B` | BC = BC − 1, 16-bit, **no flags touched** |
