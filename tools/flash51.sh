#!/bin/bash
# Build (if .asm) and flash a program to the Pt-51 (AT89C5131A) over USB DFU.
# usage: tools/flash51.sh file.asm | file.hex
set -e
SRC="$1"; [ -n "$SRC" ] || { echo "usage: $0 file.asm|file.hex"; exit 1; }
B="${SRC%.*}"
if [ "${SRC##*.}" = "asm" ]; then
  W=$(mktemp -d); N=$(basename "$B")
  python3 "$(dirname "$0")/asm2sdas.py" "$SRC" "$W/$N.s"
  (cd "$W" && sdas8051 -losgff "$N.s" && sdcc -mmcs51 "$N.rel" >/dev/null 2>&1 && packihx "$N.ihx" > "$N.hex")
  cp "$W/$N.hex" "$B.hex"; echo "built $B.hex ($(grep -c . "$B.hex") lines)"
fi
if [ "${SRC##*.}" = "c" ]; then
  W=$(mktemp -d); N=$(basename "$B")
  sed 's/#include <AT89C513xA.h>/#include <8051.h>/' "$SRC" > "$W/$N.c"
  (cd "$W" && sdcc -mmcs51 "$N.c" >/dev/null && packihx "$N.ihx" > "$N.hex")
  cp "$W/$N.hex" "$B.hex"; echo "built $B.hex from C"
fi
HEX="$B.hex"
if ! dfu-programmer at89c5131 get bootloader-version >/dev/null 2>&1; then
  echo "Pt-51 bootloader not found on USB. Put the board in boot mode:"
  echo "  1. USB switch -> DETACH   2. hold BOOT   3. tap RESET   4. release BOOT   5. USB switch -> ATTACH"
  exit 1
fi
echo "bootloader $(dfu-programmer at89c5131 get bootloader-version 2>&1 | tail -1)"
dfu-programmer at89c5131 erase --force
dfu-programmer at89c5131 flash "$HEX"
echo "flashed $HEX - press RESET on the board to run it"
