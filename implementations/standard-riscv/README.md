# Standard RISC-V Implementation

This directory contains Standard RISC-V implementations using the traditional 64-bit pointer model with `riscv64-elf-gcc` toolchain.

## Toolchain Configuration
- **Compiler**: `riscv64-elf-gcc`
- **Architecture**: `rv64imac`
- **Target**: Traditional 64-bit virtual address space
- **Memory Model**: Software-based protection

## Implementation Strategy

### **Freestanding Implementation Approach**
This research platform uses **freestanding implementations** exclusively, avoiding standard library dependencies for the following reasons:

1. **Direct CHERI Analysis**: Standard library functions mask low-level memory operations that are critical for capability analysis
2. **Consistent Comparison**: Freestanding code provides identical operation sequences between Standard RISC-V and CHERI
3. **Embedded Relevance**: Matches real-world CHERI deployment in embedded/bare-metal systems
4. **Clear Vulnerability Demonstration**: Direct memory operations show exact points where CHERI protection occurs

### **Excluded: Standard Library Variants**
We deliberately exclude `*_stdlib.c` variants because:
- Library functions like `strcpy()`, `malloc()` obscure the memory operations being analyzed
- Standard library implementations vary between platforms, reducing reproducibility
- Freestanding code better demonstrates the fundamental differences between pointer-based and capability-based memory models

## Security Test Scenarios

### 1. Buffer Overflow Attack
- **File**: `buffer_overflow.c`
- **Purpose**: Demonstrate traditional buffer overflow vulnerability
- **Assembly Analysis**: Shows lack of hardware bounds checking

### 2. Use-After-Free Vulnerability
- **File**: `use_after_free.c`
- **Purpose**: Temporal memory safety violation
- **Assembly Analysis**: No hardware protection against dangling pointers

### 3. Advanced Vulnerabilities Suite
- **File**: `advanced_vulnerabilities_freestanding.c`
- **Purpose**: Comprehensive vulnerability demonstrations including:
  - Pointer manipulation attacks
  - Array exploitation techniques
  - Function pointer hijacking
  - Stack corruption scenarios
  - Inter-object boundary violations
  - Integer overflow leading to buffer overflows
  - Double free vulnerabilities
  - Uninitialized memory access
- **Assembly Analysis**: Complete vulnerability surface analysis

## Compilation Instructions

```bash
# Standard compilation
riscv64-elf-gcc -march=rv64imac -mabi=lp64 -O2 -g -o program program.c

# Generate assembly
riscv64-elf-gcc -march=rv64imac -mabi=lp64 -S -o program.s program.c

# Analysis compilation (with debugging symbols)
riscv64-elf-gcc -march=rv64imac -mabi=lp64 -O0 -g3 -fno-omit-frame-pointer -o program_debug program.c
```

## Expected Assembly Characteristics

### Memory Access Patterns
- Direct 64-bit virtual address loading
- No bounds checking instructions
- Traditional `ld`/`sd` (load/store doubleword) instructions
- Pointer arithmetic using standard ALU operations

### Vulnerability Indicators
- Unchecked array indexing
- Direct memory address calculations
- No capability metadata
- Software-only protection boundaries

## Analysis Focus Points

1. **Memory Layout**: Traditional stack/heap organization
2. **Pointer Representation**: 64-bit virtual addresses
3. **Security Boundaries**: Software-enforced only
4. **Vulnerability Surface**: Large attack surface due to lack of hardware protection
