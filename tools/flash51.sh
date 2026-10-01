#!/bin/bash
# Build (if .asm) and flash a program to the Pt-51 (AT89C5131A) over USB DFU.
# usage: tools/flash51.sh file.asm | file.hex
set -e
SRC="$1"; [ -n "$SRC" ] || { echo "usage: $0 file.asm|file.hex"; exit 1; }
B="${SRC%.*}"
if [ "${SRC##*.}" = "asm" ]; then
  W=$(mktemp -d); N=$(basename "$B")
  python3 - "$SRC" "$W/$N.s" <<'PY'
import re, sys
out = ["\t.module t", "\t.area HOME (CODE)", "\t.area XSEG (XDATA)", "\t.area PSEG (XDATA)", "\t.area CODE (ABS)"]
for line in open(sys.argv[1]):
    line = line.split(';')[0].rstrip()
    if not line or line.strip().upper() == 'END': continue
    line = re.sub(r'\b([0-9][0-9A-Fa-f]*)H\b', lambda m: '0x' + m.group(1), line)
    if line.strip().upper().startswith('ORG'): line = '\t.org ' + line.split()[1]
    out.append(line)
open(sys.argv[2], 'w').write('\n'.join(out) + '\n')
PY
  (cd "$W" && sdas8051 -losgff "$N.s" && sdcc -mmcs51 "$N.rel" >/dev/null 2>&1 && packihx "$N.ihx" > "$N.hex")
  cp "$W/$N.hex" "$B.hex"; echo "built $B.hex ($(grep -c . "$B.hex") lines)"
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
