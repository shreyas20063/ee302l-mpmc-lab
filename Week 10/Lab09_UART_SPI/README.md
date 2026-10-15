# Lab 09 — UART password lock and SPI temperature sensor (Week 10, 15 Oct 2026)

Handout: `MPMC_Lab_09.pdf`. Both programs are in C for the Pt-51 (AT89C5131A,
**24 MHz**, X1 mode, so 1 machine cycle = 0.5 µs) and use the lab header
`<AT89C513xA.h>`. SDCC ships the same header, so they build here unchanged.

| File | Task |
|---|---|
| `q1_uart_password.c` | Q1: password over UART (PuTTY), feedback on UART, LCD and LEDs, lock after 3 wrong |
| `q2_spi_adc_lm35.c` | Q2: LM35 → MCP3008 ADC over SPI → temperature on the LCD |

## Shared pieces

- **`delay_ms`**: Timer 0 mode 1 loaded with F830H = 65536 − 2000. 2000 ticks × 0.5 µs = 1 ms. Measured in s51: `delay_ms(5000)` = 5.055 s, `delay_ms(300)` = 303 ms (the loop overhead adds about 1 %).
- **LCD**: the Lab 08 routines (RS/RW/EN on P0.0–P0.2, data on P2), with `delay_ms` in place of the busy loop. `LCD_Show` clears the screen and prints at 80H.
- **LEDs**: P1.4–P1.7 are active high, so `P1=0xF0` turns them on and `P1=0x00` turns them off.

## Q1 — UART

**Baud rate.** Timer 1 runs in mode 2 (8-bit auto-reload) as the baud clock. SMOD (PCON bit 7) = 1 doubles the rate.

```
baud = 2^SMOD / 32 × (Fosc / 12) / (256 − TH1)
     = 2 / 32 × 2 000 000 / (256 − 243) = 9615   (TH1 = F3H, 0.16 % from 9600)
```

Without SMOD there is no whole-number reload close to 9600 at 24 MHz.

The other UART registers:
- `SCON = 50H`: mode 1 (8-bit UART, start + 8 data + stop) with REN = 1 so it receives.
- `TMOD = 21H`: Timer 1 mode 2 for the baud clock, Timer 0 mode 1 for `delay_ms`.

`uart_tx` writes SBUF and waits for TI. `uart_rx` waits for RI and reads SBUF.

**Flow.**
1. Print `Enter Password:`.
2. Collect 4 characters, skipping `\r` and `\n` (requirement 7) and echoing `*` for each one kept.
3. Compare the 4 characters with "1234".
4. If it matches:
   - Print `Access Granted` and show `ACCESS OK`.
   - Turn the LEDs on, wait 5 s, turn them off.
   - Reset the wrong-attempt counter.
5. If it doesn't match:
   - Increment the counter.
   - Print `Access Denied` and show `WRONG PASSWORD`.
   - Blink the LEDs 3 times.
   - Print `Attempts remaining: n`.
6. At 3 wrong attempts:
   - Print `SYSTEM LOCKED` and show it on the LCD.
   - Turn the LEDs on.
   - Spin forever, discarding anything received, until RESET.

**Wiring.** The Pt-51's USB port belongs to the chip's bootloader, not the UART. A USB-to-UART adapter (CP2102, FT232, PL2303 or similar) connects to the P3 header:

| Adapter | Pt-51 |
|---|---|
| TX | P3.0 (RxD) |
| RX | P3.1 (TxD) |
| GND | GND |

**Terminal.** Set 9600 baud, 8N1, no flow control. In PuTTY choose Serial and the COM port. On this Mac:

```bash
screen /dev/cu.usbserial-XXXX 9600
```

Leave with Ctrl-A then K.

**Verified in s51.** The UART was exposed as a TCP port and a script typed like a user. Delays were shortened for this run only.

```
1234⏎      → Access Granted
1111⏎      → Access Denied, Attempts remaining: 2
12⏎34⏎     → Access Granted        (CR/LF ignored, counter reset)
0000⏎      → Access Denied, Attempts remaining: 2
9999⏎      → Access Denied, Attempts remaining: 1
5555⏎      → Access Denied, Attempts remaining: 0, SYSTEM LOCKED
1234       → no response (locked)
```

## Q2 — SPI ADC

**ADC: MCP3008-I/P** (10-bit, 8-channel, DIP-16), confirmed from the kit. Before the LM35, test the wiring with the pot: ends to 3.3 V and GND, wiper to CH0. The display should sweep 00.0 to 330.0 (the reading in mV / 10).

**Wiring.** The SPI header pins are fixed by the board.

| MCP3008 pin | Connect to |
|---|---|
| 16 VDD, 15 VREF | 3.3 V on the SPI header |
| 14 AGND, 9 DGND | GND |
| 13 CLK | SCK = P1.6 |
| 12 DOUT | MISO = P1.5 |
| 11 DIN | MOSI = P1.7 |
| 10 CS | SS = P1.4 |
| 1 CH0 | LM35 Vout |

| LM35 pin | Connect to |
|---|---|
| +Vs | **5 V** (the LM35 needs at least 4 V) |
| Vout | MCP3008 CH0 |
| GND | GND |

These are also the LED pins, so the LEDs flicker during transfers. The handout's note warns about exactly this sharing.

**SPI is bit-banged** in mode 0,0:
- Set MOSI, raise SCK (the ADC samples on this edge), read MISO, lower SCK (the ADC shifts on this edge).
- `spi_byte` does that 8 times, MSB first, and returns the byte read.

**MCP3008 read**, with CS low throughout:

| Send | Receive |
|---|---|
| `01H` (start bit) | — |
| `80H` (SGL = 1, channel 000) | bits 1..0 = B9 B8 |
| `00H` | B7..B0 |

**Temperature.** The LM35 gives 10 mV/°C, so the voltage in mV is the temperature in tenths of a degree:

```
mV = raw × VREF_MV / 1024          (VREF_MV = 3300; set it to 5000 if VREF is tied to 5 V)
display mV as  tt.t °C             (0xDF is the HD44780's degree symbol)
```

Resolution is 3.2 mV ≈ 0.3 °C. The reading updates every 0.5 s on LCD line 2, under `TEMPERATURE`.

**Verified on the Mac.** The real `spi_byte`/`adc_read`/display code from this file was compiled against a bit-accurate MCP3008 model (start bit, SGL/D2–D0, sample clock, null bit, B9–B0 on falling edges).

| Test | Result |
|---|---|
| ADC values 0, 77, 93, 124, 512, 1023 | all read back exactly |
| Command bits | SGL = 1, channel 0 |
| 77 counts (248 mV) | LCD shows ` 24.8°C` |
