# IR Remote Cloner using 8051

Semester project, EE302L. Point any IR remote at the board and press
**record** — it captures the signal. Press **replay** — the board becomes
that remote.

## How it works

Every IR remote sends a pattern of **marks** (38 kHz IR bursts) and
**spaces** (silence). The TSOP1738 receiver strips the 38 kHz carrier and
outputs the envelope: low during a mark, high during a space.

- **Capture:** time the length of every low/high level on the TSOP pin
  with timer 0 (0.5 µs ticks) and store the durations. No protocol
  decoding needed — we don't care if it's NEC or Sony or RC-5, we just
  replay the same shape.
- **Replay:** walk the stored durations; during marks drive the IR LED
  with a 38 kHz carrier, during spaces stay off. The receiver in the TV
  can't tell the difference.

## Pins (Pt-51)

| Signal | Pin | Note |
|---|---|---|
| TSOP1738 out | P3.2 | idles high, low during an IR mark |
| IR LED driver (transistor base) | P1.0 | Timer 2 clock-out pin; **keep onboard SW1 OFF**, or it shorts the carrier to GND |
| Receiver activity | P1.7 (LED4) | lights while the TSOP sees IR |
| Debug / control UART | P3.0 RxD, P3.1 TxD | USB-TTL converter to the Mac, 9600 8N1 (same as Lab 09) |
| LCD (later) | P2 data, P0.0–P0.2 | same as Labs 08/09 |

## Carrier: Timer 2 clock-out, not the PCA

The August plan used the PCA, but its fastest PWM is about 23 kHz and its toggle mode
would need an interrupt every 13 µs. Timer 2 has a **clock-out mode**: a hardware
50 % square wave on P1.0.

```
f = 24 MHz / (4 × (65536 − RCAP2))
```

The datasheet writes this with F_CLK_PERIPH, which suggests 12 MHz in X1 mode. **Measured
on the board it is the full 24 MHz crystal.** RCAP2 = FFB1H gave 75,950 Hz, not 38 kHz.
The `f` command measures the carrier on the board itself: a jumper from P1.0 to P3.4
lets Timer 0 count carrier edges (counter mode) for 4 PCA overflows = 131.072 ms.

| Carrier | 65536 − RCAP2 | RCAP2 | Expected | Measured |
|---|---|---|---|---|
| 36 kHz | 167 | FF59H | 35,928 Hz | |
| 38 kHz | 158 | FF62H | 37,975 Hz | **37,971 Hz** |
| 40 kHz | 150 | FF6AH | 40,000 Hz | |

Timer 2 runs all the time. A mark sets T2OE (pin = clock-out) and a space clears it,
with the P1.0 latch at 0 so the pin is driven low between marks.

Timers: T0 measures marks and spaces (0.5 µs ticks), T1 is the UART baud clock, T2
makes the carrier, and the PCA counter is a second 0.5 µs clock for the loopback.

## IR LED driver

```
 5V ──100Ω──►|── IR LED ── C
                          CL100S (NPN)    E ── GND
 P1.0 ──┬──1k───────── B
        └──2.2k── 5V
```

Port 1 drives a `1` only through a weak internal pull-up (tens of µA), which can't
switch a transistor hard enough on its own. The 2.2k pull-up supplies about 1.6 mA of
base current; when P1.0 is low the pin sinks it and the transistor turns off.
LED current is about (5 − 1.3 − 0.1) / 100 ≈ 36 mA. 100 Ω, not 47 Ω: every port pin is high
during reset and boot mode, so the LED is fully on until main() turns it off, and 47 Ω
would put 0.26 W into a ¼ W resistor during a flash.
Transistor: BC547B, flat face toward you, legs C B E. Base resistor 1k (sheet) works with
the 2.2k pull-up: about 1.3 mA of base current.

## Receiver: HS0038A2 (kit part, replaces the TSOP1738)

Vishay HS0038A2, 38 kHz, active low, 4.5–5.5 V. With the lens facing you: 1 GND,
2 VS, 3 OUT (from its datasheet). The newer HS0038**B** uses OUT/GND/VS, so check the
letter. VS through 100 Ω with 4.7 µF to GND; OUT to P3.2 (30k pull-up built in).

## Dev workflow (UART)

Until the LCD/button UI exists, the board is driven from the Mac:

```bash
cd Project && make flash        # after the BOOT sequence
screen /dev/cu.usbserial-10 9600
```

| Key | Action |
|---|---|
| `r` | wait for a remote, capture until 32 ms of silence, print every mark/space in µs |
| `p` | replay the capture |
| `d` | print the capture again |
| `t` | load a built-in NEC test frame (address 04, command 08 = LG TV power) as if it had been captured |
| `l` | loopback: transmit the stored code and record our own receiver during the same pass, print sent vs heard, count matches within 200 µs |
| `c` / `x` | carrier on / off continuously (scope check) |
| `6` `8` `0` | carrier 36 / 38 / 40 kHz |

The loopback uses the PCA counter (CMOD = 0, 0.5 µs ticks) as a second clock, because
Timer 0 is busy timing the transmission. It needs no remote and no scope: the HS0038A2
only responds near 38 kHz, so a good match proves the carrier, the LED driver, the
receiver and the capture timing in one go.

**Aim matters.** The HS0038A2 sees about ±45° off its lens. With the IR LED shining at
the receiver's side, the loopback flickered (43–75 edges heard for 67 sent). Aimed at the
lens from 10–30 cm: 67/67.

Printing captures in µs means the timing tables for the report come straight off the terminal.

## For the project sheet

Title: `IR Remote Cloner using 8051`

```
1. 8051 Development Kit - 1
2. TSOP1738 IR Receiver (38 kHz) - 1
3. IR LED, 940 nm - 2
4. BC547 NPN Transistor (or 2N2222) - 1
5. Resistors: 47 ohm - 1, 100 ohm - 1, 1k - 1, 10k - 1
6. Capacitor: 4.7 uF electrolytic - 1
7. 16x2 LCD Display (HD44780) - 1
8. Push Buttons - 2
9. Breadboard - 1
10. Jumper Wires (male-male and male-female)
11. Spare IR remote (TV/DVD) for testing - 1
```

Purpose of the passives: 47R limits IR LED current (~50 mA pulsed), 1k is
the transistor base resistor, 100R + 4.7 uF is the supply filter the TSOP
datasheet requires, 10k is a pull-up on the TSOP output. No external power
supply needed (USB 5 V is enough) and no contrast pot (trimmer is on the
Pt-51 board).

## Roadmap (full spec — all of it is committed scope, not stretch)

1. **See the signal** — TSOP wired, LED4 mirrors IR_IN. Proves receiver
   works before any capture logic.
2. **Capture** — record a burst, print it over UART. `ir_record()` is
   written; verify `n_edges` lands around 67 for a NEC remote.
3. **Carrier** — Timer 2 clock-out generating 38 kHz on P1.0,
   verified on a scope. Save the scope shot for the report.
4. **Replay** — full record → replay against a real TV/AC. First demo.
5. **Compatibility** — 150 ms silence window so one recording holds a
   whole keypress (Sony triples, AC multi-frame); carrier selectable
   36/38/40 kHz via SW3.
6. **UI** — LCD status line + 4 memory slots (SW selects slot): a working
   universal remote, not a single-code trick.
7. **Loopback self-test** — replay at our own TSOP, re-capture, diff the
   duration arrays, show match % on the LCD. The proof mechanism, and
   the demo-day party trick.
8. **NEC decoder** — when a capture parses as NEC (9 ms lead mark +
   4.5 ms space), decode and display address/command bytes on the LCD.
   Raw replay never depends on this; it is pure display value.
9. **Persistence** — store slots in the AT89C5131A's on-chip EEPROM so
   codes survive power-off (verify the EEPROM register interface in the
   datasheet; if unavailable, document volatile storage as a limitation).
10. **Report artifacts** — scope captures of carrier and envelope, timing
    tables, and the brand test matrix below.

## Compatibility plan

Three things decide whether an arbitrary remote/device pair works:

| Axis | Problem | Fix |
|---|---|---|
| Carrier | Philips = 36 kHz, NEC = 38, Sony = 40; TSOP1738 hears 36-40 (56 kHz is out of scope) | replay carrier is one PCA constant; SW3 cycles 36/38/40, LCD shows it |
| Repeats | Sony sends each keypress 3x; AC remotes send multi-frame bursts | end-of-capture silence threshold ~150 ms (count timer overflows), so one recording = the whole keypress with its gaps |
| Frame length | AC remotes exceed 100 edges | buffer is 400 edges (800 B of the 1 KB XRAM) |

**Loopback self-test:** point the IR LED at our own TSOP, replay a stored
code, re-capture it, diff the duration arrays, show match % on the LCD.
Verifies a clone with no TV needed - also the demo-day party trick.

Test matrix: one NEC-family TV remote, one Sony, one AC remote, plus
whatever else is lying around.

## Building

macOS (this repo, compile check only — flashing needs the board):

```bash
cd Project && make
```

Lab: open `main.c` in MCU 8051 IDE, compile, flash the hex with FLIP.

## Current state (22 Oct 2026)

On the Pt-51 with HS0038A2, BC547B and the clear IR LED wired as above:

| Test | Result |
|---|---|
| Carrier, `f` with P1.0→P3.4 jumper | 37,971 Hz at RCAP2 = FF62H |
| Loopback, `t` then `l` (NEC test frame, 67 durations) | 67 sent, 67 heard, **67 within 200 µs** |
| Receiver distortion in that loopback | marks +90 µs average (+37..+136), spaces −70 µs (−114..−19) |

The distortion is the receiver's own (within the HS0038A2 datasheet's pulse-width
tolerance). A captured remote carries it, so a later step should subtract about 90 µs
from marks and add about 70 µs to spaces before replay. The loopback gives the numbers.

Replay timing in s51 is within about 1 % of target (`REPLAY_FIX = 64` ticks).

Done: roadmap steps 1 (receiver), 3 (carrier, measured on the board instead of a scope),
and the core of 7 (loopback). Next: step 2 with a real remote (`r`), step 4 replay at a TV.
