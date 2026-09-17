#!/bin/bash
# Run an Intel-syntax 8051 .asm (MCU 8051 IDE / Keil style) on this Mac:
# converts it to sdas8051 syntax, assembles, runs it in s51 (ucsim), dumps internal RAM.
# usage: tools/run8051.sh file.asm [steps] [from] [to]      e.g. tools/run8051.sh q1.asm 200 0x30 0x6f
set -e
SRC="$1"; STEPS="${2:-300}"; FROM="${3:-0x30}"; TO="${4:-0x7f}"
W=$(mktemp -d); B=$(basename "${SRC%.*}")
python3 - "$SRC" "$W/$B.s" <<'PY'
import re, sys
out = ["\t.module t", "\t.area HOME (CODE)", "\t.area XSEG (XDATA)", "\t.area PSEG (XDATA)", "\t.area CODE (ABS)"]
for line in open(sys.argv[1]):
    line = line.split(';')[0].rstrip()
    if not line or line.strip().upper() == 'END': continue
    line = re.sub(r'\b([0-9][0-9A-Fa-f]*)H\b', lambda m: '0x' + m.group(1), line)
    if line.strip().upper().startswith('ORG'):
        line = '\t.org ' + line.split()[1]
    out.append(line)
open(sys.argv[2], 'w').write('\n'.join(out) + '\n')
PY
cd "$W"
sdas8051 -losgff "$B.s" && sdcc -mmcs51 "$B.rel" >/dev/null 2>&1
printf "step $STEPS\ndump iram $FROM $TO\nquit\n" | s51 -t 8051 "$B.ihx" 2>/dev/null | grep -E "^(0x[0-9a-f]{2} |Stop at|     R0|     [0-9a-f]{2} |@R0)"
