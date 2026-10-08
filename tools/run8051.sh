#!/bin/bash
# Run an Intel-syntax 8051 .asm (MCU 8051 IDE / Keil style) on this Mac:
# converts it to sdas8051 syntax, assembles, runs it in s51 (ucsim), dumps internal RAM.
# usage: tools/run8051.sh file.asm [steps] [from] [to]      e.g. tools/run8051.sh q1.asm 200 0x30 0x6f
set -e
SRC="$1"; STEPS="${2:-300}"; FROM="${3:-0x30}"; TO="${4:-0x7f}"
W=$(mktemp -d); B=$(basename "${SRC%.*}")
python3 "$(dirname "$0")/asm2sdas.py" "$SRC" "$W/$B.s"
cd "$W"
sdas8051 -losgff "$B.s" && sdcc -mmcs51 "$B.rel" >/dev/null 2>&1
printf "step $STEPS\ndump iram $FROM $TO\nquit\n" | s51 -t 8051 "$B.ihx" 2>/dev/null | grep -E "^(0x[0-9a-f]{2} |Stop at|     R0|     [0-9a-f]{2} |@R0)"
