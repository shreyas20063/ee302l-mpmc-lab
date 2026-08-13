# Lab 1 — 8085 Arithmetic (13/08/2026)

Official handout title: *"Lab 1 Questions – 13/08/2026"*. Three tasks, all
assembling at `7000H`.

| Task | File | Story | Core instruction |
|---|---|---|---|
| 1 | `task1_sensor_add.asm` | add two sensor readings | `ADD M` |
| 2 | `task2_offset_sub.asm` | reference − measured, observe borrow | `SUB M` |
| 3 | `task3_counter_add16.asm` | add two 16-bit counts + carry out | `ADC` |

## Memory maps

**Tasks 1 & 2** — operands `7501H`, `7502H` → result `7503H`.

**Task 3** — little-endian 16-bit values:

| Address | Contents |
|---|---|
| `7601H` / `7602H` | count 1, low / high |
| `7603H` / `7604H` | count 2, low / high |
| `7500H` / `7501H` | sum, low / high |
| `7502H` | carry beyond 16 bits (`00H` or `01H`) |

## Expected results

| Task | Data | Result | Flags to observe |
|---|---|---|---|
| 1 | `19H + 1CH` | `7503H = 35H` | CY = 0 |
| 2 | `40H − 2EH` | `7503H = 12H` | CY = 0, no borrow |
| 2 (swapped) | `2EH − 40H` | `7503H = EEH` | **CY = 1, borrow** — `EEH` = two's complement of `12H` |
| 3 | `80FFH + 9001H` | `7500H = 00H`, `7501H = 11H`, `7502H = 01H` | both carries fire |

Task 3's data is deliberately chosen so the low-byte addition also carries
(`FFH + 01H`); that proves `ADC` genuinely added the carry rather than the
high bytes just happening to sum right.

## Running

1. Paste the task into the Editor, **Assemble**, set `Start From` to `7000`.
2. **Step By Step** → **Forward**; watch the flag panel — Task 2's entire
   point is seeing CY at the `SUB` step, both with and without borrow.
3. Read results in the **Memory** tab.
4. Between tasks, note that memory persists: Task 3's output region
   (`7500H–7502H`) overwrites Tasks 1/2's data area. Reassembling loads
   fresh values, or clear memory from the Settings tab.

## Viva ammunition

- Why does the carry survive between `ADD` and `ADC`? Because `LDA`, `STA`,
  `MOV`, `MVI` don't affect flags. That property *is* multi-byte arithmetic.
- Faster alternative for Task 3: `LHLD 7601H` / `XCHG` / `LHLD 7603H` /
  `DAD D` / `SHLD 7500H` — `DAD` does the whole 16-bit add in one
  instruction (and touches *only* CY). The byte-wise `ADC` version is shown
  instead because it generalises to any width; knowing both is worth marks.
- Task 2: why is `EEH` "correct"? Subtraction that borrows returns the
  result modulo 256 — the two's complement. CY is the sign information.
