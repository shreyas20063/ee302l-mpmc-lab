# EE302L — Microprocessor and Microcontroller Lab

Sem 5, IIT Dharwad. Instructor: Prof. Samba Raju Chiluveru.

Coursework for both halves of the syllabus:

- **8085 microprocessor** — assembly, run in Jubin Mitra's 8085 Simulator
- **8051 microcontroller** — assembly and C, on the Pt-51 board (Atmel AT89C5131A)

## Layout

```
Week N/<experiment>/     source, plus a README with memory map and results
Project/                 semester project: IR Remote Cloner (see Project/README.md)
tools/                   toolchain + build files (see tools/README.md)
tools/templates/         starting points for new 8051 programs
Reference/               course deck and datasheets
```

One folder per lab week, one sub-folder per experiment.

## Progress

| Week | Experiment | Status |
|---|---|---|
| 1 | 8085 — add two 8-bit numbers | done, verified in simulator |
| 1 | 8085 — addition with carry propagation | written, to verify in sim |
| 1 | 8051 — LED blink (Assembly + C) | not started |
| 2 | Lab 1 T1 — sensor addition (8-bit) | done, verified in simulator |
| 2 | Lab 1 T2 — offset subtraction, borrow | done, verified in simulator |
| 2 | Lab 1 T3 — 16-bit counter addition | done, verified in simulator |
| 3 | Lab 3 E1 — multiply by repeated addition | written, to verify in sim |
| 3 | Lab 3 E2 — divide by repeated subtraction | written, to verify in sim |
| 3 | Lab 3 E3 — safe-limit detection (branching) | written, to verify in sim |
| 4 | ADC threshold filter (arrays, CPI/STAX) | written, to verify in sim |
| 4 | LED pyramid (computed, OUT, CALL/RET, delay) | done, verified in simulator |
| 4 | Traffic light controller (table-driven, auto-stop) | done, verified in simulator |
| 5 | Q1 — array reversal via PUSH/POP | written, to verify in sim |
| 5 | Q2 — subroutine add, BC preserved on stack | written, to verify in sim |
| 5 | Q3 — BCD counter + RST 7.5 ISR | done, verified at the bench |
| 6 | Lab 05 T1 — railway crossing (RST 7.5, IN/OUT) | done, verified in simulator |
| 6 | Lab 05 T2 — tank level controller (classify, average, RST 7) | written, to verify in sim |

## Getting set up on a new machine

The toolchain here is macOS-specific because the course material targets
Ubuntu/Windows. See [tools/README.md](tools/README.md) for what's installed,
what has no macOS equivalent, and the assembly-dialect differences that matter.

Quick check that everything is present:

```bash
make -f tools/Makefile.8051 check
```
