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
