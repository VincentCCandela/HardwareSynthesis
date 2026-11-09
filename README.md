# Hardware Synthesis

This project is a C++ hardware synthesis tool. It reads a sequence of arithmetic assignments and hardware resource constraints, converts them to a static single assignment (SSA), creates a directed acyclic graph (DAG) representing the dependencies, schedules the operations with several strategies (ASAP, ALAP, resource constrained, FDS), and derives datapath control information such as registers, multiplexers, and sequencer events.

## Repository organization

The project is currently organized as a single-source-directory repository:

```text
.
├── main.cpp       # Command-line entry point and overall pipeline
├── parse.h/.cpp  # Input parsing and SSA conversion
├── schedule.h/.cpp# Dependency DAG and operation scheduling
├── datapath.h/.cpp
│                  # Register, multiplexer, and sequencer information generation
├── input.txt/input2.txt      # Example arithmetic input

```

## Synthesis tool architecture

The executable runs the following pipeline:

1. **Parsing and SSA conversion** (`parse.cpp`)
   - Reads arithmetic assignments such as `X = A + B`, only `+`, `-`, and `*` are supported.
   - Renames repeated assignments and intermediate values using digits to create SSA forms, such as `X0 = A0 + B0`.
   - Tracks the available number of `+`, `-`, and `*` resources defined by the user.

2. **Dependency graph construction** (`schedule.cpp`)
   - Converts the SSA nodes into an adjacency matrix.
   - Stores the operation type on each matrix diagonal entry.

3. **Operation scheduling** (`schedule.cpp`)
   - Computes ASAP and ALAP schedules assuming unlimited resources.
   - Computes a force-directed schedule (FDS) that balances resource
     utilization across cycles.
   - Computes resource-constrained ASAP and ALAP schedules based on user-defined available units.

4. **Datapath generation** (`datapath.cpp`)
   - Inserts registers required between dependent operations based on DAG.
   - Groups operation inputs into multiplexers, with at most one multiplexer per hardware unit. We assume that the multiplexers can take any number of input.
   - Produces per-cycle register and multiplexer events for the sequencer FSM.

`main.cpp` prints the intermediate graph, each schedule, and the generated
datapath/sequencer information.


## Build


The repository includes a `Makefile` for the project build configuration, can build with

```bash
make
```

To remove generated build outputs:

```bash
make clean
```

## Run

Run the tool with an input file:

```bash
./main input.txt
```

To also export the SSA representation to `ssa.txt`, pass
`--export-ssa` as the second argument:

```bash
./main input.txt --export-ssa
```
## Input format

Each non-empty input line should contain a binary arithmetic assignment using
`+`, `-`, or `*`, for example:

```text
X = A + B
Y = X + A
Z = X - B
Q = B * Z
```

Resource counts may be listed after the assignments, for example:

```text
+:1
-:1
*:1
```

## Outputs
The program prints:

- The node information (operation type and output name)
- SSA format in text file when requested
- The dependency adjacency matrix
- ASAP, ALAP, force-directed, constrained ASAP, and constrained ALAP schedules with (output name, cycle) pairs as well as total cycle time required
- Register list with register input, output, and time enabled information
- Multiplexer list for each hardware unit with mux input names and the time each input is selected 
- Sequencer events grouped by cycle, specifically at each cycle how each reg and mux are configured

## Tasks 5-8: Verilog and the Terasic C5G

```text
├── verilog.h/.cpp # Verilog generator for one schedule
├── hdl/fu.v       # 8-bit adder, subtractor and multiplier
├── hdl/top.v      # C5G top level: buttons, switches, 7-segment display
├── quartus/       # Quartus Prime project for the C5G (5CGXFC5C6F27C7)
└── generated/     # generator output for the example under Input format (asap)
```

`make` compiles as C++23; the code only uses C++17 features.

```bash
./main input.txt --verilog asap   # or alap, fds, casap, calap
```

This writes `generated/hls_core.v`, the synthesized design: a control-step counter (the sequencer), one operand multiplexer per functional unit, the task-4 registers, and exactly the adders, subtractors and multipliers the schedule uses. It also writes `generated/testbench.v`, which checks every operation's result on its unit in its cycle, and prints the unit numbers and the expected unit outputs for the hardcoded inputs.

The datapath is 8 bits: one value fits on SW7-SW0 and two hex digits show it, and results wrap modulo 256. Inputs start at hardcoded values (2, 3, 4, ... in order of first use); the first four can be changed from the switches. Each value shows on its unit during the cycle that computes it.

Simulate:

```bash
iverilog -o tb.vvp generated/testbench.v generated/hls_core.v hdl/fu.v && vvp tb.vvp
```

Compile with Quartus Prime Lite, which runs on Linux or Windows but not macOS:

```bash
cd quartus
quartus_sh --flow compile hls
quartus_cpf --option=bitstream_compression=off -c output_files/hls.sof output_files/hls.rbf
```

Flash from a Mac with [openFPGALoader](https://github.com/trabucayre/openFPGALoader) (`brew install openfpgaloader`), with the board's USB cable plugged into the Mac, the power on and the RUN/PROG switch on RUN:

```bash
scp <user>@<linux-host>:HardwareSynthesis/quartus/output_files/hls.rbf .
openFPGALoader -b c5g hls.rbf
```

On the machine with Quartus, `quartus_pgm -m jtag -o "p;output_files/hls.sof"` from `quartus/` flashes it instead.

### Board controls

Each switch is 0 when down (toward the edge of the board) and 1 when up. A group of switches is read as a binary number with SW0 as the ones place: SW0 = 1, SW1 = 2, SW2 = 4, SW3 = 8, SW4 = 16, SW5 = 32, SW6 = 64, SW7 = 128. `./main` writes these groups as SW[4:0], SW[7:0] and SW[9:8].

| Control | Function |
|---|---|
| KEY0 | go to the next cycle |
| KEY1 | load the number on SW7-SW0 into the input numbered by SW9-SW8 |
| KEY4 (CPU_RESET_n) | back to cycle 0 with the hardcoded inputs |
| SW4-SW0 | number of the unit shown on HEX1-HEX0 |
| HEX3-HEX2 | current cycle |
| HEX1-HEX0 | output of the selected unit in hex; 00 if it computes nothing this cycle |

Units are numbered adders first, then subtractors, then multipliers, and inputs in order of first use; `./main` prints both lists (`SW[4:0] unit:` and `SW[9:8] input:`). In the example under Input format, unit 0 is the adder (SW4-SW0 all down), 1 the subtractor (only SW0 up) and 2 the multiplier (only SW1 up); input 0 is A (SW9 and SW8 down) and 1 is B (only SW8 up).

SW4-SW0 change the display as soon as they move; the switches are read for a load only when KEY1 is pressed. Because SW4-SW0 are part of both, the display can show another unit, or 00, while a value is set. Load inputs after KEY4 and before the first KEY0, since KEY4 restores the hardcoded inputs.

Example with A = 7 and B = 4 (only the listed switches are up):

| Step | Switches up | Press | Display | Meaning |
|---|---|---|---|---|
| 1 | none | KEY4 | 0005 | A = 2, B = 3 |
| 2 | SW2, SW1, SW0 | KEY1 | 0000 | A = 4 + 2 + 1 = 7 |
| 3 | SW8, SW2 | KEY1 | 0000 | B = 4 |
| 4 | none | | 000b | adder, cycle 0: X = 7 + 4 = 11 |
| 5 | none | KEY0 | 0112 | adder, cycle 1: Y = 11 + 7 = 18 |
| 6 | SW0 | | 0107 | subtractor, cycle 1: Z = 11 - 4 = 7 |
| 7 | SW1 | KEY0 | 021C | multiplier, cycle 2: Q = 4 * 7 = 28 |
| 8 | none | KEY0 | 0300 | done |

## 30-minute demo plan

| Time | Plan |
|---|---|
| 0-5 | Repository layout and pipeline: parse, SSA, DAG, schedules, datapath, Verilog |
| 5-10 | Example under Input format: `./main input.txt --export-ssa`, then `./main input.txt --verilog asap`; walk through `generated/hls_core.v` and run the testbench |
| 10-13 | Board with the example: step with KEY0, pick units with SW4-SW0, compare with the printed values |
| 13-28 | Three runs with the professor's equations, 5 minutes each |
| 28-30 | Questions |

Each run:

1. The professor writes the equations (and optional `+:N -:N *:N` limits) into a file and picks a schedule.
2. `./main file --verilog <schedule>` prints the schedules and the expected unit outputs and writes the Verilog; run the testbench.
3. Compile with Quartus and flash with openFPGALoader (about 2-3 minutes) while explaining `generated/hls_core.v`.
4. Optionally load other inputs (see Board controls) and work out their results by hand.
5. Step through the cycles with KEY0 and check each unit's output on HEX1-HEX0.

Before the demo: run `make`, compile the example once in Quartus, and confirm `openFPGALoader -b c5g --detect` finds the board.
