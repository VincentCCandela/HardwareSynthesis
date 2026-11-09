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

The datapath is 8 bits: one value fits on SW[7:0] and two hex digits show it, and results wrap modulo 256. Inputs start at hardcoded values (2, 3, 4, ... in order of first use); the first four can be changed from the switches. Each value shows on its unit during the cycle that computes it.

Simulate:

```bash
iverilog -o tb.vvp generated/testbench.v generated/hls_core.v hdl/fu.v && vvp tb.vvp
```

Program the board: open `quartus/hls.qpf` in Quartus Prime Lite, compile, and download `output_files/hls.sof` with the Programmer (RUN/PROG switch on RUN). From `quartus/` on the command line: `quartus_sh --flow compile hls`, then `quartus_pgm -m jtag -o "p;output_files/hls.sof"`.

| Control | Function |
|---|---|
| KEY[0] | step one control step |
| KEY[1] | load SW[7:0] into input SW[9:8] |
| SW[4:0] | functional unit shown on HEX1-HEX0 |
| CPU_RESET_n | back to cycle 0 with the hardcoded inputs |
| HEX3-HEX2 | current cycle |
| HEX1-HEX0 | output of the selected unit (hex) |

## 30-minute demo plan

| Time | Plan |
|---|---|
| 0-5 | Repository layout and pipeline: parse, SSA, DAG, schedules, datapath, Verilog |
| 5-10 | Example under Input format: `./main input.txt --export-ssa`, then `./main input.txt --verilog asap`; walk through `generated/hls_core.v` and run the testbench |
| 10-13 | Board with the example: step with KEY[0], switch units with SW[4:0], compare with the printed values |
| 13-28 | Three runs with the professor's equations, 5 minutes each |
| 28-30 | Questions |

Each run:

1. The professor writes the equations (and optional `+:N -:N *:N` limits) into a file and picks a schedule.
2. `./main file --verilog <schedule>` prints the schedules and the expected unit outputs and writes the Verilog; run the testbench.
3. Compile and program in Quartus (about 2-3 minutes) while explaining `generated/hls_core.v`.
4. Optionally load other inputs (SW[9:8], SW[7:0], KEY[1]) and work out their results by hand.
5. Step through the cycles with KEY[0] and check each unit's output on HEX1-HEX0.

Before the demo: run `make`, compile the example once in Quartus, and confirm the Programmer finds the board.
