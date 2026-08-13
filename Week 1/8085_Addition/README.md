# Week 1 — Addition of two 8-bit numbers (8085)

## Files

| File | What it does |
|---|---|
| `add_8bit.asm` | The assigned exercise. Stores the 8-bit sum only. |
| `add_8bit_carry.asm` | Extension: full 16-bit result. **Has a TODO to complete.** |

## Memory map

| Address | Contents |
|---|---|
| `7000H` | program |
| `7501H` | first operand |
| `7502H` | second operand |
| `7503H` | sum, low byte |
| `7504H` | sum, high byte (carry version only) |

## Running it

1. Launch the simulator — double-click `tools/8085 Simulator.command`.
2. *File → Open* the `.asm`, or paste it into the Editor tab.
3. Click **Assemble**.
4. Switch to the **Assembler** tab. **Set `Start From` to `7000`** — it
   defaults to `0000`. Then **Step By Step** and use **Forward**.
5. Read the result in the **Memory** tab at `7503H`. The Registers panel shows
   the accumulator and flags after each step.

Data placed by `# DB` appears in the **Memory** tab, not the Assembler
Workspace. An empty workspace does not mean the data failed to load.

### Set `Start From` to 7000, not 0000

Leaving it at `0000` still produces the right answer, but only by accident:
blank memory reads as `00`, which is `NOP`, so the CPU runs 28,672 NOPs before
reaching `7000H`. You can see it in the counters — Instruction Counter reads
`28679` (= 28672 + 7) and Clock Cycle Counter `114743` (= 28672 × 4 + 55).

Once two programs are loaded at different ORGs, running from `0000` will
execute whichever it reaches first.

## Expected results

| Operands | `add_8bit.asm` → 7503H | `add_8bit_carry.asm` → 7503H / 7504H |
|---|---|---|
| `12H`, `13H` | `25H` | `25H` / `00H` |
| `88H`, `88H` | `10H` — **carry lost** | `10H` / `01H` |

## Expected assembly

`add_8bit.asm` should assemble to 9 bytes starting at `7000H`:

| Address | Bytes | Mnemonic |
|---|---|---|
| 7000 | `21 01 75` | `LXI H, 7501` |
| 7003 | `7E` | `MOV A, M` |
| 7004 | `23` | `INX H` |
| 7005 | `86` | `ADD M` |
| 7006 | `23` | `INX H` |
| 7007 | `77` | `MOV M, A` |
| 7008 | `CF` | `RST 1` |

If your Assembler tab shows different hex, the source was mistyped.

## Verified run

`add_8bit.asm` with `12H, 13H`, confirmed in the simulator:

| Readout | Value |
|---|---|
| Accumulator | `25` |
| `Memory(M)` / `7503H` | `25` |
| Memory Pointer (HL) | `7503` |
| CY flag | `0` (no overflow) |
| PC at halt | `0008` — `RST 1` vector |
| SP at halt | `FFFE` — `RST 1` pushed a return address |

## Watch out

- **Hex is the default** number base — `7501` means `7501H`. Decimal literals
  need a `d` suffix *and* at least 5 digits (`0015d`, not `15d`).
- **Do not use `DAA` here.** The simulator has a documented bug (manual §1.4)
  where `DAA` toggles the carry flag instead of setting it when a carry is
  already present. It doesn't affect this program, but it will bite on any
  BCD experiment later.
- Don't put a `;` comment after a `//` comment on the same line — known
  preprocessor bug.
