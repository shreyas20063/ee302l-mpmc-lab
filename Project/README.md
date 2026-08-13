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
| TSOP1738 out | P3.2 | idles high, active low |
| IR LED (via transistor) | P1.4 | shared with onboard LED1 — it doubles as a transmit indicator |
| Record button | P1.0 | onboard SW1 |
| Replay button | P1.1 | onboard SW2 |
| Busy light | P1.7 | onboard LED4 |

38 kHz carrier plan: PCA high-speed output on CEX1 (= P1.4), so the
hardware generates the carrier and the CPU only gates it on/off. PCA
registers come from the AT89C5131A datasheet (not in sdcc's 8051.h).

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
2. **Capture** — record a burst, show edge count. `ir_capture()` is
   written; verify `n_edges` lands around 67 for a NEC remote.
3. **Carrier** — PCA high-speed output generating 38 kHz on P1.4,
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

## Current state

Scaffold compiles (sdcc 4.6.0). Capture and replay logic written but
untested on hardware; `carrier_on()` is a placeholder until the PCA is
configured. Buffer sits in the chip's 1 KB on-chip XRAM
(`--xram-size 1024`).
