# Lab 07 — 8051 timers and software PWM (Week 8)

Handout: `MPMC_Lab_07.pdf`. Target: Pt-51 board (AT89C5131A) and MCU 8051 IDE.
All programs assume a 12 MHz clock, so one machine cycle = 12 clocks = 1 µs.

| File | Task |
|---|---|
| `blink_p1.asm`, `blink_p1.c` | Lab assignment from the deck: blink the four LEDs on P1.4–P1.7 |
| `ex1_timer0_pattern.asm` | Ex 1: Timer 0 mode 1, new LED pattern every 1 s (0001→0010→0100→1000) |
| `ex2_timer1_toggle.asm` | Ex 2: Timer 1 mode 1, toggle P1.4 every 1 s while switch on P1.3 is on |
| `ex3_pwm_p20.asm` | Ex 3: ≈1 kHz PWM on P2.0, 25/50/75 % duty, no timers or interrupts |

## Board wiring that matters

| Pin | On the Pt-51 |
|---|---|
| P1.4–P1.7 | LED1–LED4, active high (deck writes F0H to light them) |
| P1.0–P1.3 | SW1–SW4 slide switches |
| P2.0 | free header pin — put the oscilloscope probe here for Ex 3 |

## One second from a 16-bit timer

Mode 1 counts up from the value loaded into TH/TL and sets TF when it
rolls over from FFFFH to 0000H. The longest single count is 65 536 µs, so
1 s is built from 20 overflows of 50 000 µs each:

```
65536 − 50000 = 15536 = 3CB0H   →  TH = 3CH, TL = B0H
20 × 50 ms = 1 s                →  R2 = 14H
```

`DELAY1S` reloads 3CB0H, starts the timer, spins on `JNB TFx` until it
overflows, stops and clears the flag, and repeats 20 times. The reload and
loop overhead add about 11 µs per second (measured 1.000229 s in s51).

TMOD picks the timer and mode: `01H` = Timer 0 mode 1 (Ex 1),
`10H` = Timer 1 mode 1 (Ex 2). Nothing else in TMOD is touched.

## Ex 1 — rotating pattern

`A` holds the pattern (10H, 20H, 40H, 80H). `MOV P1,A` drives the LEDs,
`RL A` moves the 1 one bit left. After 80H an `RL` gives 01H, which is the
cue to reset to 10H. Expected on the board: one LED lit at a time, walking
LED1→LED2→LED3→LED4 and back, one step per second.

## Ex 2 — switch-gated toggle

`SETB P1.3` makes the pin an input, `CLR P1.4` starts with the LED off.
The main loop waits while P1.3 reads 1 (`JB P1.3,MAIN`), then delays 1 s and
`CPL P1.4` flips the LED. So while the switch is on the LED changes state once
per second; when it is off the LED freezes in whatever state it had.

If the switch on your board reads **1** when on, change `JB` to `JNB`. In the
IDE's simulator click the P1.3 bit in the port panel to act as the switch.

## Ex 3 — software PWM

The period is 4 slots of ≈250 µs. The duty is the number of HIGH slots,
kept in RAM **30H** (1, 2 or 3 — edit that line for 25 %, 50 %, 75 %).

`SLOT` is a 120-lap `DJNZ` loop: ACALL 2 + MOV 1 + 120×2 + RET 2 = 245 µs,
plus the `DJNZ R1` that follows each call = 247 µs per slot. Measured in s51
at 50 %: HIGH 499 µs, LOW 499 µs, period 998 µs → **1.002 kHz**.

| 30H | HIGH slots | LOW slots | Duty |
|---|---|---|---|
| 01H | 1 | 3 | 25 % |
| 02H | 2 | 2 | 50 % |
| 03H | 3 | 1 | 75 % |

Only 1–3 are valid: `DJNZ` with a count of 0 would run 256 slots.

## Blink assignment

Nested 255×255 `DJNZ` delay. Measured 130.8 ms per half period, so the LEDs
blink at about 3.8 Hz. The C version is the deck's program; the header
`<AT89C513xA.h>` is the one MCU 8051 IDE ships for this chip.

## Verification

All four were checked headlessly with s51 (uCsim) at 12 MHz using
breakpoints on the port writes and the "Simulated … sec" readout:

| Program | Observed |
|---|---|
| Ex 1 | P1 = 10, 20, 40, 80, 10 … with 1.000 s between writes |
| Ex 2 | with P1.3 held low, P1.4 toggles every 1.000 s |
| Ex 3 (50 %) | 499 µs high, 499 µs low |
| Blink | 130.8 ms on, 130.8 ms off |

In MCU 8051 IDE: open the `.asm`, click the rocket (assemble + start
simulator), then **Virtual HW → LED Panel**, set the four LEDs to port 1 bits
4–7 and switch the panel ON, then Simulator → Run (F9). For Ex 3 use the
Simulator → Stopwatch to time the P2.0 edges, or read the Time field.

## Flashing to the Pt-51 (lab PC)

1. FLIP: Device → Select → AT89C5131.
2. USB switch to DETACH, hold BOOT, tap RESET, release BOOT, switch to ATTACH.
3. FLIP: Settings → Communication → USB → Open. Status shows "USB ON".
4. File → Load HEX file → the `.hex` the IDE generated.
5. Run (Erase, Blank check, Program, Verify all green), then press RESET.

## Flashing from this Mac (no FLIP needed)

The AT89C5131A boot loader speaks USB DFU, which `dfu-programmer` (Homebrew)
supports directly. `.hex` files here were built with `sdas8051` + `packihx`
and are plain Intel HEX, the same format FLIP loads.

```bash
tools/flash51.sh "Week 8/Lab07_Timers_PWM/ex1_timer0_pattern.asm"
```

Board sequence first (same as for FLIP): USB switch DETACH → hold BOOT →
tap RESET → release BOOT → USB switch ATTACH. The script checks for the
boot loader, erases, flashes, and tells you to press RESET.

**Clock caveat:** the exercises say 12 MHz, but the Pt-51 has a **24 MHz**
crystal. In the default (X1) mode one machine cycle is then 0.5 µs, so on the
board every delay runs twice as fast: Ex 1/Ex 2 step every 0.5 s, Ex 3 comes
out at ≈2 kHz. The submitted code follows the 12 MHz spec; for true 1 s and
1 kHz on the board change `MOV R2,#14H` → `#28H` (40 overflows) and
`MOV R0,#78H` → `#0F0H` (240 laps per slot).
