# Lab 08 — LCD interfacing and switch nibble (Week 9, 08 Oct 2026)

Handouts: `MPMC_Lab_08_Questions.pdf`, `LCD_Interfacing_Pt51.pdf`.
Display: JHD162A (16×2, HD44780U controller) plugged into the Pt-51 LCD slot.

| File | Task |
|---|---|
| `q1_lcd_char_int.asm` / `.c` | Q1: a character at the right end of line 1, an integer at the left end of line 2 |
| `q2_lcd_name_date.asm` / `.c` | Q2: name on line 1, date on line 2, both centred |
| `q3_switch_nibble.asm` | Q3: read SW1–SW4 as a nibble, show it on LED1–LED4, store it in 60H |

## Wiring (fixed by the board)

| Signal | Pin | Meaning |
|---|---|---|
| RS | P0.0 | 0 = the byte on P2 is a command, 1 = it is a character |
| RW | P0.1 | 0 = write (always, here) |
| EN | P0.2 | the LCD latches P2 on the **high→low** edge |
| D0–D7 | P2 | the byte |

Every transfer is the same four steps: put the byte on P2, set RS, pulse EN
high then low, wait. `LCDCMD` and `LCDDATA` differ only in RS.

## Init sequence (what the four commands mean)

| Byte | Command | Bits |
|---|---|---|
| 38H | Function set | 0011 **1**000: 8-bit bus, **2** lines, 5×7 font |
| 0CH | Display switch | 0000 1**1**00: display on, cursor off, blink off |
| 01H | Clear screen | also homes the cursor |
| 06H | Entry mode | 0000 01**1**0: cursor moves right after each character |

Power-on needs ~15 ms before the first command; the programs call `DELAY`
three times (≈7.5 ms at 24 MHz) — the deck's single call also works on the
bench. `DELAY` is a 10×255 `DJNZ` ≈ 5 ms at 12 MHz / 2.5 ms at 24 MHz, far
longer than the 1.52 ms the clear command needs.

## Cursor addressing

A "set DDRAM address" command is `80H + position`:

| | Column 0 | … | Column 15 |
|---|---|---|---|
| Line 1 | 80H | | 8FH |
| Line 2 | C0H | | CFH |

- Q1: `8FH` → right end of line 1, then `'S'`; `C0H` → left end of line 2, then `7 + 30H = '7'`.
- Q2: `Shreyas & Yash` is 14 chars → (16 − 14) / 2 = 1 → `81H`. `08 OCT 2026` is 11 → (16 − 11) / 2 = 2 → `C2H`.

Digits are sent as ASCII: add 30H to the number. Q1 does this explicitly
(`ADD A,#30H`); Q2 stores the text as a `DB` string ending in 0 and `PRINT`
walks it with `MOVC A,@A+DPTR` until it hits the 0.

## Q3

```
MOV P1,#0FH      low nibble latches = 1 so SW1–SW4 can be read
MOV A,P1
CPL A            switches read 0 when ON (verified in Lab 07), invert so ON = 1
ANL A,#0FH       keep the nibble
MOV 60H,A        store
MOV C,ACC.0      copy each switch bit to the LED at the other end
MOV P1.7,C       SW1 -> LED4 ... SW4 -> LED1, so the LEDs mirror the DIP block
MOV C,ACC.1
MOV P1.6,C
MOV C,ACC.2
MOV P1.5,C
MOV C,ACC.3
MOV P1.4,C
SJMP MAIN
```

Bit moves leave P1.0–P1.3 untouched, so the switch pins stay readable.

If the TA wants the raw pin reading (ON = 0) instead, delete `CPL A`.

## Verification (s51, breakpoints on LCDCMD/LCDDATA)

| Program | Bytes seen on P2 |
|---|---|
| Q1 | cmd 38 0C 01 06 8F, data `S`, cmd C0, data `7` |
| Q2 | cmd 38 0C 01 06 81, data `Shreyas & Yash`, cmd C2, data `08 OCT 2026` |
| Q3 | switches 1010 ON → 60H = 0A, LEDs show the mirrored 0101 |

All three ran on the Pt-51 on 08 Oct 2026 (flashed over DFU from this Mac).

Both C files compile with SDCC (header swapped to `<8051.h>`; the lab IDE
uses `<AT89C513xA.h>` — same `P0_0` pin names). Flash from this Mac with
`tools/flash51.sh file.asm` or `file.c` after the boot sequence
(DETACH → hold BOOT → tap RESET → release BOOT → ATTACH). If the LCD shows
nothing, turn the blue contrast pot next to the switches.
