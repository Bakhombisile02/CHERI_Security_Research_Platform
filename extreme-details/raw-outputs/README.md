# Raw Outputs and Logs

This directory contains unprocessed, raw outputs from compilation, execution, and analysis of both Standard RISC-V and Authentic CHERI implementations.

## Directory Structure

```
raw-outputs/
├── standard-riscv/           # Raw outputs from Standard RISC-V
│   ├── compilation-logs/     # Compiler output and warnings
│   ├── assembly-dumps/       # Raw assembly output files
│   ├── execution-logs/       # Runtime execution traces
│   ├── gdb-sessions/        # Complete GDB debugging sessions
│   └── memory-dumps/        # Memory layout snapshots
├── authentic-cheri/         # Raw outputs from Authentic CHERI
│   ├── compilation-logs/     # CHERI-LLVM compiler output
│   ├── assembly-dumps/       # Raw CHERI assembly output
│   ├── execution-logs/       # CHERI runtime traces
│   ├── capability-dumps/     # Capability state snapshots
│   └── protection-logs/     # Hardware protection violation logs
└── comparison-data/         # Side-by-side raw comparison data
    ├── size-comparisons/    # Binary size analysis
    ├── performance-logs/    # Execution time measurements
    └── instruction-counts/  # Instruction frequency analysis
```

## Raw Data Collection Process

### Standard RISC-V Raw Data Collection

#### 1. Compilation Logs
```bash
# Capture complete compiler output with maximum verbosity
riscv64-elf-gcc -march=rv64imac -mabi=lp64 -v -save-temps -time -Q \
  -fverbose-asm -Wa,-acdlns -O2 -g3 program.c 2>&1 | tee compilation.log

# Capture preprocessor output
riscv64-elf-gcc -E -dM program.c > preprocessor_defines.log

# Capture optimization details
riscv64-elf-gcc -march=rv64imac -mabi=lp64 -O2 -fdump-tree-all \
  -fdump-rtl-all -fdump-ipa-all program.c 2>&1 | tee optimization.log
```

#### 2. Assembly Dumps
```bash
# Complete assembly with annotations
riscv64-elf-gcc -march=rv64imac -mabi=lp64 -S -fverbose-asm \
  -Wa,-acdlns -o program_annotated.s program.c

# Objdump disassembly with symbols
riscv64-elf-objdump -d -S -l --source program > disassembly_full.log

# Detailed section analysis
riscv64-elf-objdump -h -t -r program > sections_symbols.log
```

#### 3. Execution Traces
```bash
# QEMU execution trace
qemu-riscv64 -d in_asm,cpu,exec,int -D execution_trace.log program

# Instruction-level tracing
qemu-riscv64 -singlestep -d in_asm,cpu -D instruction_trace.log program

# Memory access tracing
qemu-riscv64 -d guest_errors,mmu -D memory_trace.log program
```

### Authentic CHERI Raw Data Collection

#### 1. CHERI Compilation Logs
```bash
# Complete CHERI-LLVM output
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap \
  -v -save-temps -time -Xclang -ast-dump -O2 -g3 program.c 2>&1 | tee cheri_compilation.log

# Capability ABI details
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap \
  -mllvm -print-before-all -mllvm -print-after-all program.c 2>&1 | tee abi_details.log

# LLVM pass information
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap \
  -mllvm -debug-pass=Structure program.c 2>&1 | tee llvm_passes.log
```

#### 2. CHERI Assembly Dumps
```bash
# CHERI assembly with capability annotations
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap \
  -S -fverbose-asm -o program_cheri_annotated.s program.c

# CHERI objdump with capability information
/Users/dlaba556/cheri/output/sdk/bin/llvm-objdump -d -S --mattr=+xcheri \
  program > cheri_disassembly_full.log

# Capability relocation information
/Users/dlaba556/cheri/output/sdk/bin/llvm-objdump -r --mattr=+xcheri \
  program > capability_relocations.log
```

#### 3. CHERI Execution Traces
```bash
# QEMU-CHERI with capability tracing
qemu-system-riscv64cheri -d in_asm,cpu,exec,cap -D cheri_execution_trace.log

# Capability instruction tracing
qemu-system-riscv64cheri -d cap,int -D capability_trace.log

# Memory protection violation logs
qemu-system-riscv64cheri -d guest_errors,mmu,cap -D protection_violations.log
```

## Raw Log Analysis Guidelines

### Critical Information to Extract

#### From Standard RISC-V Logs
- Virtual address calculations
- Memory access patterns (ld/sd instructions)
- Stack frame layout
- Function call conventions
- Pointer arithmetic operations
- Security vulnerability indicators

#### From CHERI Logs
- Capability derivation chains
- Bounds checking operations
- Capability metadata preservation
- Protection violation triggers
- Capability instruction usage patterns
- Spatial/temporal safety enforcement

### Log Processing Pipeline

1. **Raw Collection**: Capture all output with maximum verbosity
2. **Categorization**: Sort logs by compilation/execution/debugging phase
3. **Filtering**: Extract security-relevant information
4. **Cross-Reference**: Link assembly instructions to source code
5. **Comparative Analysis**: Identify protection mechanism differences

## File Naming Convention

### Standard RISC-V
- `{test_name}_riscv_compilation_{timestamp}.log`
- `{test_name}_riscv_assembly_{optimization_level}.s`
- `{test_name}_riscv_execution_{timestamp}.trace`

### Authentic CHERI
- `{test_name}_cheri_compilation_{timestamp}.log`
- `{test_name}_cheri_assembly_{mode}_{optimization_level}.s`
- `{test_name}_cheri_execution_{timestamp}.trace`

### Comparison Data
- `{test_name}_comparison_{metric}_{timestamp}.log`
- `size_comparison_{test_name}_{timestamp}.csv`
- `performance_comparison_{test_name}_{timestamp}.csv`

## Data Integrity

All raw outputs are preserved exactly as generated with:
- Complete timestamps
- Tool version information
- Command-line parameters used
- Environment configuration details
- Hardware/software context

This ensures reproducibility and allows for historical analysis of toolchain evolution and optimization improvements.
