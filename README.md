# Compiler IR 
This is an IR representation model for developing and testing optimizations and codegen technics.
### Structure
```
├── include/  ---------- Ir main structures and helper funcs for creating them
│   ├── BasicBlock.h
│   ├── common.h
│   ├── Graph.h
│   ├── Instruction.h
├── tests/  ---------- Unit tests with examples of code
│   ├── basic_tests.cpp
│   ├── CMakeLists.txt
```
### Building
Run the following commands:
```
$ cmake -S . -B build
$ cmake --build build
```
### Running and Testing
Compiler executable is useless, use unit tests. To launch unit tests run the following command:
```
$ cd build && ctest
```
### Implementation
The IR has the following implementation:
#### Instruction
- `op`           : Opcode
- `type`         : Type of instruction (e.g. u32, u64)
- `prev`         : Previous instruction
- `next`         : Next instruction
- `args`         : Virtual regs as arguments
- `id`           : Target and label ids
- `bb`           : BasicBlock that contains this instruction
To build an instruction pass op, type, args, id to ctor. Also prev and next can be passed to ctor or can be set up by setters.
#### BasicBlock
- `first_instr`  : Pointer to the first instruction in the basic block
- `last_instr`   : Pointer to the last instruction in the basic block
- `first_phi`    : Pointer to the first phi instruction in the basic block
- `preds`        : Vector of predecessors to basic block
- `succs`        : Vector of successors to basic block
- `graph`        : Pointer to current graph
To build a basic block pass first_instr and last_instr to ctor or set them up by setters.
#### Graph
- `root`         : Pointer to the beginning basic block in the graph
- `end`          : Pointer to the ending basic block in the graph
- `params_num`   : Number of function parameters
- `label_table`  : Table of label targets for indexing
- `jump_table`   : Table of targets as `Instruction` pointers
- `vreg_table`   : Table of virtual regs values
To build a graph pass root, end and params_num parameters to ctor.
