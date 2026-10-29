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
| Receiver activity | P1.7 (LED4) | lights while the receiver sees IR |
| Slot select | P1.1–P1.3 (SW2–SW4) | 3-bit slot number, SW2 = MSB, ON = 1: 000 → slot 1 … 111 → slot 8 |
| SEND button | P3.5 | push button to GND (internal pull-up). Press = read SW2–SW4 and send that slot; hold > 1 s = send it again 6 s later |
| REC button | P3.3 | press and release = record into the selected slot (press again to cancel); hold 3 s = erase it. A slot recorded this way is named by its code, e.g. `8355-90` |
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

**Perfboard version (29 Oct 2026).** Powering the soldered board from the Pt-51's 5V/GND
with P1.0 not yet wired dimmed the LCD; unplugging the IR LED cured it. With P1.0 floating
the 2.2k pull-up holds the transistor on, so the LED draws current continuously. Once the
whole circuit, P1.0 included, was connected and main() drove `IR_OUT` low, the LCD stayed
bright and record/send worked. Check: 3.4–3.7 V across the 100 Ω means 36 mA (correct);
about 0 V across it while the LCD dims means the resistor is bypassed.

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
| `r` | record a button (capture ends after 164 ms of silence). If it decodes as NEC, it asks for a slot 1–8 and a name and saves it to EEPROM |
| `1`–`8` | send that slot: rebuilt NEC frame + 40 ms gap + one repeat pulse (one 108 ms period, like a real short press) |
| `L` | list slots (name, address, command) |
| `e` + digit | erase a slot |
| `p` / `o` | replay the last raw capture / the same twice, 6 s apart |
| `d` | print the last capture in µs |
| `t` / `l` | load the NEC test frame / loopback self-test |
| `f` | measure the carrier (needs a jumper from P1.0 to P3.4) |
| `k` | timing correction on/off for raw replay |
| `q` | carrier 36 → 38 → 40 kHz |
| `c` / `x` | carrier on / off continuously |

### Standalone control

**Verified on the board (22 Oct 2026):** both LCD lines, slots recorded with REC, and the
projector responding to SEND **repeatedly**. One apparent failure was aim. The 5 mm IR LED's
beam is narrow (about ±20°) and the projector's sensor is directional, so a few degrees off
at 1 m loses it. The earlier "same code ignored within 3 s" result may partly have been aim
as well; repeated SENDs of rebuilt frames (with the repeat pulse) work when aimed properly.
More LED current (a second 100 Ω in parallel, about 72 mA) would widen the usable angle and range.

The switches are read only when the button is pressed, so the number can be changed freely
in between, and one press sends exactly once. The LCD shows the selection live
(`SEL 3: ENTER` / `IR 38kHz  7/8 SAVED`). SW1 (P1.0) cannot take part: P1.0 is the only
Timer 2 clock-out pin, and SW1 ON grounds it.

### LCD

Same wiring and driver as Labs 08/09 (data P2, RS/RW/EN on P0.0–P0.2). `delay_ms` borrows
Timer 0 (F830H = 1 ms), so the LCD only updates between captures and replays, never inside
a timing loop. Every command shows its result: `IR CLONER 38kHz` / `SLOTS n/8 USED` when
idle, `RECORDING...`, `NEC 8355 CMD 90` / `SAVE TO SLOT 1-8`, the name as it is typed,
`SENT SLOT 2` / `SOURCE`, `MATCH 67 OF 67`, `37971 Hz`. Lines are padded to 16 characters
instead of clearing the screen, so it doesn't flicker. EN is held high for 1 ms (as in
Lab 09): EN is on P0, which has only a slow external pull-up, and a 0.5 µs pulse made the LCD
miss the first 38H, leaving it in 1-line mode (line 2 blank). 38H is now sent twice after a
50 ms wait. In s51, startup writes 38 0C 01 06,
then `IR CLONER 38kHz` and `SLOTS 0/8 USED`.

### Slots and EEPROM

NEC buttons are stored decoded: address (2 bytes) + command (the inverted check byte is
recomputed), 4 + 9 bytes per slot with the name. 8 slots = 104 bytes in EEPROM page 0
(the chip has 1 KB of data EEPROM in 128-byte pages). To write: set EEE in EECON, MOVX the
bytes into the page latches, write 52H then A2H to EECON, and wait for EEBUSY to clear. To
read: EEE plus AUXR.M0 (stretched MOVX). A blank EEPROM reads FFH, so the used flag must be
exactly 01H. The slots are loaded at boot and survive resets, power-off and probably
reflashing (DFU erase targets the flash).

Replaying a rebuilt frame instead of the raw capture gives textbook NEC timing, free of the
receiver's +90/−70 µs distortion, and the repeat pulse makes it a complete keypress. In s51
the NEC test frame builds to 71 durations: frame, 40,000 µs gap, 9000/2250/560 repeat,
frame + gap = 107.98 ms; it decodes back to 04 FB 08 F7. The EEPROM path cannot be
simulated; checked on the board.

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
and the core of 7 (loopback). 
**First real clone (22 Oct 2026): Epson projector, POWER button.**

| | |
|---|---|
| Capture | 67 durations, NEC: lead 9161/4360 µs, bursts 670–697 µs, gaps 400–427 / 1525–1561 µs |
| Decoded | `83 55 90 6F`: extended NEC address 83 55 (Epson), command 90, check byte OK |
| Replay | raw replay (no correction), IR LED aimed at the projector's IR window from under 1 m: **projector responded** |

Lessons from the session:
- A stray IR flash near the projector lamp started one capture (1 duration, 314 µs). The
  recorder now ignores captures shorter than 8 durations and keeps waiting.
- Captures live in XRAM and are lost when the board resets or loses power (roadmap step 9).
- Epson asks for confirmation on power-off, so one POWER replay is a safe, visible test.

**Off and on, same session.** Epson asks "Power off?" on the first POWER press and needs a
second press to confirm. Two replays 1 s or 3 s apart only showed the dialog; **6 s apart
turned it off**. The projector ignores the same code again within a few seconds (a
lock-out against reflections and double triggers); a person's double press is naturally
slower. After the cool-down, one replay turned it back on. The `o` key does the 6 s
double press.

Problems hit and fixed during the session:
- Garbled UART (`1MM&�679…`) after re-plugging the converter: its GND wire must go
  straight to a Pt-51 GND pin. The board is powered from another PC, so that wire is the
  only shared reference.
- Each replay showed the dialog but never confirmed; first suspected the missing NEC repeat
  pulses, then found the lock-out above.

### Capture format (current `main.c`)

Durations under 16.4 ms are stored in 0.5 µs ticks. Longer gaps (between frames, before
repeat pulses) are stored as `8000H | ticks/64` (32 µs steps, up to 1 s). Capture ends after
5 Timer 0 overflows (164 ms) of silence. Replay waits in 30 ms chunks for long gaps.
`REPLAY_FIX` re-measured at 212 ticks for the 32-bit wait routine. In s51, a planted
9000/4500/560/40000/560 µs code replays as 8999/4500/559/40050/556 µs. The loopback
compares the first frame only.

This build (164 ms capture, `o`, `REPLAY_FIX` 212) is verified in s51, not yet flashed.
The board still runs the build that did the off/on above.

Next: flash this build, range test, then the UI (LCD and buttons) and memory slots.
