# Authentic CHERI Implementation

This directory contains Authentic CHERI implementations using real capability-based protection with the CHERI-LLVM compiler.

## Toolchain Configuration
- **Compiler**: CHERI-LLVM (`/Users/dlaba556/cheri/output/sdk/bin/clang`)
- **Architecture**: `rv64imafdcxcheri`
- **Target**: CHERI-RISC-V with 128-bit capabilities
- **Memory Model**: Hardware capability-based protection

## Implementation Strategy

### **Freestanding Implementation Approach**
This research platform uses **freestanding implementations** exclusively for CHERI analysis:

1. **Pure Capability Demonstration**: Freestanding code shows raw CHERI capability instructions without library abstraction
2. **Direct Protection Analysis**: Manual memory operations demonstrate exactly where CHERI hardware protection occurs
3. **Consistent Comparison**: Identical operation sequences to Standard RISC-V enable precise capability vs pointer comparison
4. **Hardware Feature Focus**: Direct capability manipulation shows CHERI's fundamental protection mechanisms

### **Excluded: Standard Library Variants**
We deliberately exclude `*_stdlib.c` variants because:
- CHERI-enhanced library functions mask the fundamental capability operations we're analyzing
- Standard library implementations hide the low-level capability derivation and bounds setting
- Freestanding code better demonstrates CHERI's hardware protection at the instruction level
- Direct capability manipulation shows the precise security boundaries CHERI enforces

## CHERI Security Mechanisms

### Capability Structure (128-bit)
- **Base Address**: 64-bit base pointer
- **Length**: Object size/bounds
- **Permissions**: Read/Write/Execute/Load-Cap/Store-Cap
- **Type**: Software-defined type information
- **Tag**: Hardware validity bit

### Hardware Protection Features
- **Spatial Safety**: Automatic bounds checking on every memory access
- **Temporal Safety**: Capability revocation and invalidation
- **Type Safety**: Software-defined type system support
- **Compartmentalization**: Capability-based isolation

## Security Test Scenarios (CHERI-Protected)

### 1. Buffer Overflow Protection
- **File**: `buffer_overflow_cheri.c`
- **Protection**: Hardware bounds checking prevents overflow
- **Assembly**: `clc`/`csc` instructions with automatic bounds verification

### 2. Use-After-Free Protection
- **File**: `use_after_free_cheri.c`
- **Protection**: Capability invalidation prevents temporal violations
- **Assembly**: Capability clearing on deallocation

### 3. Advanced Protection Suite
- **File**: `advanced_protections_cheri_freestanding.c`
- **Protection**: Comprehensive CHERI protection mechanisms including:
  - Pointer manipulation protection via capability bounds
  - Array access protection through automatic bounds checking
  - Function pointer protection via sealed capabilities
  - Stack protection through capability stack frames
  - Inter-object protection via precise capability bounds
  - Temporal safety through capability invalidation
  - Integer overflow protection via capability arithmetic
  - Allocator protection through capability-based memory management
- **Assembly**: Complete CHERI capability instruction analysis
- **Protection**: Capability-based heap allocation
- **Assembly**: `malloc` returns bounded capabilities

### 6. Integer Overflow Protection
- **File**: `integer_overflow_cheri.c`
- **Protection**: Bounds checking prevents overflow exploitation
- **Assembly**: Capability arithmetic with overflow detection

### 7. Double Free Protection
- **File**: `double_free_cheri.c`
- **Protection**: Capability invalidation prevents reuse
- **Assembly**: Capability clearing prevents double-free

## Compilation Instructions

```bash
# CHERI compilation (pure capability mode)
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap -O2 -g -o program program.c

# Generate CHERI assembly
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap -S -o program.s program.c

# Hybrid mode compilation (capabilities + integers)
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-hybrid -O2 -g -o program program.c

# Analysis compilation (with capability debugging)
/Users/dlaba556/cheri/output/sdk/bin/clang --config cheribsd-riscv64-purecap -O0 -g3 -fno-omit-frame-pointer -o program_debug program.c
```

## Expected Assembly Characteristics

### CHERI-Specific Instructions
- **`clc` (Capability Load)**: Load 128-bit capability with tag checking
- **`csc` (Capability Store)**: Store capability with tag propagation
- **`cincoffset`**: Add offset to capability while preserving bounds
- **`csetbounds`**: Set capability bounds (narrowing only)
- **`cgetbase`/`cgetlen`**: Extract capability metadata
- **`ctag`**: Check capability tag validity

### Memory Access Patterns
- Capability-based addressing instead of virtual addresses
- Automatic bounds checking on every memory access
- Tag bit validation for capability authenticity
- Metadata preservation through capability operations

### Protection Mechanisms
- **Spatial Protection**: Hardware prevents out-of-bounds access
- **Temporal Protection**: Capability invalidation prevents use-after-free
- **Type Protection**: Software-defined capability types
- **Compartment Protection**: Capability-based isolation

## Analysis Focus Points

1. **Capability Metadata**: 128-bit capability structure analysis
2. **Hardware Enforcement**: Automatic protection mechanisms
3. **Performance Impact**: Capability overhead measurement
4. **Security Guarantees**: Formal protection properties

## CHERI Advantages

### Spatial Memory Safety
- **Problem Solved**: Buffer overflows, array bounds violations
- **Mechanism**: Hardware bounds checking on every access
- **Assembly Evidence**: Bounds checks in `clc`/`csc` instructions

### Temporal Memory Safety
- **Problem Solved**: Use-after-free, double-free vulnerabilities
- **Mechanism**: Capability invalidation and revocation
- **Assembly Evidence**: Capability clearing instructions

### Fine-Grained Protection
- **Problem Solved**: Coarse-grained page-based protection
- **Mechanism**: Object-level capability granularity
- **Assembly Evidence**: Per-object capability derivation

### Compartmentalization
- **Problem Solved**: Monolithic address spaces
- **Mechanism**: Capability-based isolation boundaries
- **Assembly Evidence**: Capability domain switching
