# Step-by-Step Analysis Framework

This directory contains detailed, instruction-by-instruction analysis of how Standard RISC-V and Authentic CHERI handle security-critical operations.

## Analysis Methodology

### 1. Source-to-Assembly Mapping
- **Goal**: Understand how high-level security concepts translate to machine code
- **Process**: Line-by-line correlation between C source and assembly output
- **Focus**: Security-relevant operations and their protection mechanisms

### 2. Execution Flow Analysis
- **Goal**: Step through program execution to understand runtime behavior
- **Process**: Instruction-by-instruction execution trace analysis
- **Focus**: Memory access patterns and protection enforcement

### 3. Protection Mechanism Deep Dive
- **Goal**: Understand exactly how each architecture enforces memory safety
- **Process**: Detailed analysis of security enforcement instructions
- **Focus**: Vulnerability prevention and attack surface reduction

## Directory Structure

```
step-by-step-analysis/
├── buffer-overflow/          # Buffer overflow scenario analysis
│   ├── standard-riscv/      # Traditional vulnerable implementation
│   ├── authentic-cheri/     # CHERI-protected implementation
│   └── comparison.md        # Side-by-side analysis
├── use-after-free/          # Use-after-free scenario analysis
├── pointer-arithmetic/      # Pointer arithmetic safety analysis
├── stack-overflow/          # Stack-based overflow analysis
├── heap-corruption/         # Heap manipulation analysis
├── integer-overflow/        # Integer overflow to memory corruption
├── double-free/            # Double-free vulnerability analysis
└── methodology/            # Analysis framework documentation
```

## Standard RISC-V Analysis Template

### Source Code Breakdown
```c
// Example: Buffer overflow scenario
char buffer[8];
strcpy(buffer, argv[1]);  // Potentially unsafe copy
printf("Buffer content: %s\n", buffer);
```

### Assembly Analysis Steps

#### Step 1: Stack Frame Setup
```assembly
# Function prologue
addi    sp, sp, -32           # Allocate 32 bytes on stack
sd      ra, 24(sp)           # Save return address
sd      s0, 16(sp)           # Save frame pointer
addi    s0, sp, 32           # Set up frame pointer

# Analysis: Traditional stack frame, no bounds information
# Security Implication: Stack layout vulnerable to overflow
```

#### Step 2: Buffer Allocation
```assembly
# Local buffer allocation
addi    a0, s0, -16          # Calculate buffer address (s0-16)
mv      a1, a0               # Buffer address for strcpy

# Analysis: Simple pointer arithmetic, 8-byte buffer at s0-16
# Security Implication: No bounds checking, buffer size unknown to hardware
```

#### Step 3: Memory Operation
```assembly
# strcpy operation (simplified)
call    strcpy               # Call strcpy function

# Analysis: Direct memory copy without bounds checking
# Security Implication: Can overflow buffer, corrupt stack frame
```

#### Step 4: Memory Access
```assembly
# Buffer access for printf
addi    a1, s0, -16          # Load buffer address
call    printf               # Print buffer content

# Analysis: Direct memory access using calculated address
# Security Implication: No verification of buffer integrity
```

### Vulnerability Analysis
- **Attack Vector**: Overwrite buffer[8] to corrupt stack frame
- **Impact**: Can overwrite return address, achieve code execution
- **Protection**: None - purely software-based stack canaries at best

## Authentic CHERI Analysis Template

### Source Code (Identical)
```c
// Same source code, different protection mechanisms
char buffer[8];
strcpy(buffer, argv[1]);  // Now CHERI-protected
printf("Buffer content: %s\n", buffer);
```

### CHERI Assembly Analysis Steps

#### Step 1: Capability Stack Frame
```assembly
# Function prologue with capabilities
cincoffset  csp, csp, -32    # Adjust capability stack pointer
csc         cra, 24(csp)     # Store return capability
csc         cs0, 16(csp)     # Store frame capability
cmove       cs0, csp         # Set up frame capability

# Analysis: Capability-based stack management
# Security Implication: Stack pointer is bounded capability, prevents stack overflow
```

#### Step 2: Bounded Buffer Capability
```assembly
# Create bounded capability for buffer
cincoffset  ca0, cs0, -16    # Calculate buffer base address
csetbounds  ca0, ca0, 8      # Set exact bounds: 8 bytes only
cmove       ca1, ca0         # Buffer capability for strcpy

# Analysis: Hardware creates 8-byte bounded capability
# Security Implication: Hardware enforces exact buffer bounds
```

#### Step 3: Protected Memory Operation
```assembly
# CHERI-aware strcpy
call        strcpy           # Call CHERI strcpy implementation

# Internal strcpy behavior:
# clc   ct0, 0(ca1)         # Load capability-based character
# csc   ct0, 0(ca0)         # Store with capability bounds check
# cincoffset ca0, ca0, 1    # Increment within bounds
# cincoffset ca1, ca1, 1    # Bounds checked automatically

# Analysis: Every memory access verified against capability bounds
# Security Implication: Hardware prevents overflow at instruction level
```

#### Step 4: Verified Memory Access
```assembly
# Protected buffer access
cmove       ca1, cs0         # Get buffer capability (still bounded)
cincoffset  ca1, ca1, -16    # Calculate buffer address
call        printf           # Print with bounds-checked access

# Analysis: Buffer access uses original bounded capability
# Security Implication: Hardware guarantees buffer integrity
```

### Protection Analysis
- **Attack Prevention**: Hardware bounds checking prevents overflow
- **Capability Metadata**: 128-bit capability contains base, length, permissions
- **Automatic Enforcement**: No software overhead, hardware-enforced protection

## Comparative Analysis Framework

### Instruction-Level Comparison

| Operation | Standard RISC-V | Authentic CHERI | Security Difference |
|-----------|----------------|-----------------|-------------------|
| Buffer Allocation | `addi a0, sp, -16` | `cincoffset ca0, csp, -16; csetbounds ca0, ca0, 8` | CHERI adds precise bounds |
| Memory Store | `sd t0, 0(a0)` | `csc ct0, 0(ca0)` | CHERI checks bounds automatically |
| Pointer Arithmetic | `addi a0, a0, 8` | `cincoffset ca0, ca0, 8` | CHERI preserves and checks bounds |
| Function Call | `call func` | `ccall func` | CHERI manages capability permissions |

### Performance Impact Analysis

#### Instruction Count Comparison
- **Standard RISC-V**: ~15 instructions for buffer operations
- **Authentic CHERI**: ~18 instructions for buffer operations
- **Overhead**: ~20% instruction increase for comprehensive protection

#### Memory Overhead
- **Standard RISC-V**: 8 bytes per pointer
- **Authentic CHERI**: 16 bytes per capability (128-bit)
- **Overhead**: 2x memory usage for pointer metadata

#### Security Benefit
- **Standard RISC-V**: Vulnerable to spatial/temporal attacks
- **Authentic CHERI**: Hardware-guaranteed spatial/temporal safety
- **Trade-off**: Performance cost for comprehensive security

## Analysis Documentation Standards

### Step Documentation Format
1. **Source Context**: Relevant C code section
2. **Assembly Instruction**: Exact assembly instruction
3. **Semantic Analysis**: What the instruction accomplishes
4. **Security Implication**: How this affects security
5. **Attack Surface**: Potential vulnerabilities or protections

### Comparison Methodology
1. **Functional Equivalence**: Ensure both implementations achieve same goal
2. **Instruction Mapping**: Map equivalent operations across architectures
3. **Protection Analysis**: Identify security mechanism differences
4. **Performance Assessment**: Measure overhead of protection mechanisms

This framework enables deep understanding of how architectural differences translate to real-world security improvements.
