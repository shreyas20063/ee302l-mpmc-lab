# MPMC Lab toolchain (macOS)

The course deck assumes Ubuntu/Windows. This is the macOS equivalent.

## 8085 half — works fully on this Mac

| | |
|---|---|
| Simulator | `8085Compiler.jar` (Jubin Mitra's 8085 Simulator v2) |
| Source | <https://github.com/8085simulator/8085simulator> |
| Launch | double-click `tools/8085 Simulator.command`, or `java -jar tools/8085Compiler.jar` |

Identical to the lab's version — it's the same JAR, and Java is cross-platform.
Write `.asm`, *File → Save Assembly language code → Save Mnemonics*, then
*Assemble* with the start address and step through.

## 8051 / Pt-51 half — builds here, flashing needs the board

| Tool | Deck's choice | Here | Status |
|---|---|---|---|
| C compiler | SDCC (via MCU 8051 IDE) | `sdcc` 4.6.0 | works |
| Assembler | MCU 8051 IDE | `sdas8051` | works, **different syntax** |
| IDE + simulator | MCU 8051 IDE | MCU 8051 IDE 1.4.9 in Docker | works, see below |
| Flasher | Atmel FLIP | `dfu-programmer` 1.1.0 | installed, untested on hardware |

### Building

```bash
make -f ../../tools/Makefile.8051 SRC=led_blink.c      # or .asm
make -f ../../tools/Makefile.8051 check                # verify toolchain
make -f ../../tools/Makefile.8051 SRC=led_blink.c flash
```

Both paths were verified end-to-end and produce valid Intel HEX.
Templates for each are in `tools/templates/`.

### The assembly syntax trap

### MCU 8051 IDE in Docker

No macOS build exists, so the real IDE runs in an Ubuntu 24.04 desktop container
(`tiryoh/ros2-desktop-vnc:jazzy` + `apt install mcu8051ide`), shown in the browser
over noVNC. Double-click `tools/MCU 8051 IDE.command`; this folder appears inside
at `/home/ubuntu/lab`, so the IDE edits the repo files directly.

Rebuild from scratch:

```bash
docker run -d --name mcu8051ide -p 6080:80 --shm-size=512m -v "$PWD":/home/ubuntu/lab tiryoh/ros2-desktop-vnc:jazzy
docker exec mcu8051ide bash -c 'apt-get update && apt-get install -y mcu8051ide'
```

`sdas8051` uses **ASxxxx** syntax; MCU 8051 IDE uses **Intel/ASEM-51**. The
deck's Week-1 listing fails to assemble here verbatim — five errors on `org`,
`end`, and the `#0F0H` literal form.

| | MCU 8051 IDE (lab) | sdas8051 (here) |
|---|---|---|
| Origin | `org 100h` | `.org 0x0100` |
| Hex literal | `#0F0H` | `#0xF0` |
| Terminator | `end` | *(omit)* |
| SFR names | built in | define, e.g. `P1 = 0x90` |
| Areas | implicit | declare `HOME`, `XSEG`, `PSEG` |

**C is portable between both** apart from the header: SDCC uses `<8051.h>`,
MCU 8051 IDE uses `<AT89C513xA.h>`.

If your submission must be the `.asm` file itself, write it in the lab's
dialect and build it on a lab machine. Use this Mac setup for C, or keep two
copies.

### Flashing with dfu-programmer

`dfu-programmer` lists `at89c5131` as a supported target. It exists precisely
because FLIP couldn't flash over USB off Windows. Enter bootloader mode exactly
as the deck describes — USB switch to DETACH, hold BOOT, tap RESET, release
BOOT, switch to ATTACH — then:

```bash
dfu-programmer at89c5131 erase
dfu-programmer at89c5131 flash led_blink.hex
```

Press RESET to run. **Not yet tested against a real board** — verify before
relying on it for a submission deadline. macOS may also require approving the
USB device on first connect.
