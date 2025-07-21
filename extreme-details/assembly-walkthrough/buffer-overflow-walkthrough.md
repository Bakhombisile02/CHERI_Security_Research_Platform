# Buffer Overflow: Complete Assembly Walkthrough

## Overview
This document provides a line-by-line assembly analysis of buffer overflow scenarios comparing Standard RISC-V and Authentic CHERI implementations.

## Source Code Context
```c
void vulnerable_function(char *user_input) {
    char buffer[8];  // Small buffer - vulnerable to overflow
    strcpy(buffer, user_input);
    printf("Buffer content: %s\n", buffer);
}
```

## Standard RISC-V Assembly Walkthrough

### Function Prologue (Stack Frame Setup)
```assembly
vulnerable_function:
    addi    sp, sp, -32          # Step 1: Allocate 32 bytes on stack
    sd      ra, 24(sp)           # Step 2: Save return address at sp+24
    sd      s0, 16(sp)           # Step 3: Save frame pointer at sp+16
    addi    s0, sp, 32           # Step 4: Set frame pointer to top of stack
```

#### Detailed Analysis:
**Step 1: Stack Allocation**
- **Instruction**: `addi sp, sp, -32`
- **Operation**: `sp = sp - 32`
- **Purpose**: Allocates 32 bytes of stack space for local variables
- **Memory Layout**: Creates new stack frame below current frame
- **Security Implication**: ❌ No bounds checking on stack allocation
- **Vulnerability**: Stack overflow possible with deep recursion

**Step 2: Return Address Save**
- **Instruction**: `sd ra, 24(sp)`
- **Operation**: `memory[sp + 24] = ra`
- **Purpose**: Saves return address for function exit
- **Location**: Stored 8 bytes from top of frame (64-bit address)
- **Security Implication**: ❌ Return address vulnerable to corruption
- **Attack Vector**: Buffer overflow can overwrite return address

**Step 3: Frame Pointer Save**
- **Instruction**: `sd s0, 16(sp)`
- **Operation**: `memory[sp + 16] = s0`
- **Purpose**: Saves previous frame pointer
- **Location**: Stored 16 bytes from frame base
- **Security Implication**: ❌ Frame pointer can be corrupted
- **Attack Vector**: Stack-based overflow can chain frame corruption

**Step 4: Frame Setup**
- **Instruction**: `addi s0, sp, 32`
- **Operation**: `s0 = sp + 32`
- **Purpose**: Establishes frame pointer for local variable access
- **Reference Point**: Points to top of current frame
- **Security Implication**: ❌ No validation of frame integrity

### Local Buffer Allocation
```assembly
    addi    a0, s0, -16          # Step 5: Calculate buffer address
    mv      a1, a0               # Step 6: Copy buffer address to a1
```

#### Detailed Analysis:
**Step 5: Buffer Address Calculation**
- **Instruction**: `addi a0, s0, -16`
- **Operation**: `a0 = s0 - 16`
- **Purpose**: Calculate address of 8-byte local buffer
- **Memory Location**: Buffer located 16 bytes below frame pointer
- **Security Implication**: ❌ No bounds metadata associated with pointer
- **Vulnerability**: Pointer arithmetic unchecked

**Step 6: Address Copy**
- **Instruction**: `mv a1, a0`
- **Operation**: `a1 = a0`
- **Purpose**: Prepare buffer address for strcpy call
- **Data Type**: Raw 64-bit virtual address
- **Security Implication**: ❌ No indication of buffer size

### Vulnerable Memory Copy
```assembly
    call    strcpy               # Step 7: Call strcpy function
```

#### strcpy Implementation (Conceptual):
```assembly
strcpy_loop:
    lb      t0, 0(a1)            # Load byte from source
    beq     t0, zero, strcpy_end # Check for null terminator
    sb      t0, 0(a0)            # Store byte to destination ← NO BOUNDS CHECK
    addi    a1, a1, 1            # Increment source pointer
    addi    a0, a0, 1            # Increment destination pointer
    j       strcpy_loop          # Continue loop
strcpy_end:
    sb      zero, 0(a0)          # Store null terminator ← MAY OVERFLOW
```

#### Detailed Analysis:
**Critical Vulnerability Points:**
1. **No Destination Bounds**: strcpy has no knowledge of destination buffer size
2. **Unchecked Store**: `sb t0, 0(a0)` stores without bounds verification
3. **Pointer Increment**: `addi a0, a0, 1` can move beyond buffer bounds
4. **Stack Corruption**: Overflow can overwrite saved registers and return address

### Function Epilogue
```assembly
    ld      ra, 24(sp)           # Step 8: Restore return address
    ld      s0, 16(sp)           # Step 9: Restore frame pointer
    addi    sp, sp, 32           # Step 10: Deallocate stack frame
    ret                          # Step 11: Return to caller
```

#### Vulnerability in Epilogue:
- **Corrupted Return Address**: If buffer overflow overwrote `memory[sp+24]`
- **Control Flow Hijack**: `ret` instruction jumps to attacker-controlled address
- **Code Execution**: Attacker gains arbitrary code execution

## Authentic CHERI Assembly Walkthrough

### Function Prologue (Capability Stack Frame)
```assembly
vulnerable_function:
    cincoffset  csp, csp, -64    # Step 1: Adjust capability stack pointer
    csc         cra, 48(csp)     # Step 2: Store return capability
    csc         cs0, 32(csp)     # Step 3: Store frame capability
    cincoffset  cs0, csp, 64     # Step 4: Set capability frame pointer
```

#### Detailed Analysis:
**Step 1: Capability Stack Adjustment**
- **Instruction**: `cincoffset csp, csp, -64`
- **Operation**: `csp.offset = csp.offset - 64`
- **Purpose**: Allocate 64 bytes on capability stack
- **Capability Preservation**: Maintains csp bounds and permissions
- **Security Advantage**: ✅ Stack bounds preserved and checked

**Step 2: Return Capability Save**
- **Instruction**: `csc cra, 48(csp)`
- **Operation**: `capability_memory[csp + 48] = cra` (128-bit store)
- **Purpose**: Save return capability (not just address)
- **Security Advantage**: ✅ Return capability is sealed and authenticated
- **Protection**: Hardware prevents forging return capabilities

**Step 3: Frame Capability Save**
- **Instruction**: `csc cs0, 32(csp)`
- **Operation**: `capability_memory[csp + 32] = cs0` (128-bit store)
- **Purpose**: Save previous frame capability
- **Security Advantage**: ✅ Frame integrity protected by capability system

**Step 4: Frame Capability Setup**
- **Instruction**: `cincoffset cs0, csp, 64`
- **Operation**: `cs0 = csp + 64` (with bounds preservation)
- **Purpose**: Establish frame capability for local access
- **Security Advantage**: ✅ Frame access bounded by capability limits

### CHERI Buffer Allocation with Bounds
```assembly
    cincoffset  ca0, cs0, -16    # Step 5: Calculate buffer base address
    csetbounds  ca0, ca0, 8      # Step 6: Set exact bounds (8 bytes)
    cmove       ca1, ca0         # Step 7: Copy bounded capability
```

#### Detailed Analysis:
**Step 5: Buffer Base Calculation**
- **Instruction**: `cincoffset ca0, cs0, -16`
- **Operation**: `ca0 = cs0 - 16` (capability arithmetic)
- **Purpose**: Calculate buffer location within frame
- **Security Advantage**: ✅ Capability arithmetic preserves original bounds

**Step 6: Bounds Setting (Critical Security Feature)**
- **Instruction**: `csetbounds ca0, ca0, 8`
- **Operation**: Creates capability with exactly 8-byte bounds
- **Capability Structure**: 
  - Base: Buffer start address
  - Length: Exactly 8 bytes
  - Permissions: Read/Write
  - Tag: Valid (1)
- **Security Advantage**: ✅ Hardware enforces exact buffer bounds

**Step 7: Capability Copy**
- **Instruction**: `cmove ca1, ca0`
- **Operation**: Copy bounded capability to argument register
- **Purpose**: Prepare bounded buffer capability for function call
- **Security Advantage**: ✅ Bounds travel with pointer

### CHERI-Protected Memory Copy
```assembly
    ccall       strcpy           # Step 8: Call CHERI-aware strcpy
```

#### CHERI strcpy Implementation:
```assembly
cheri_strcpy_loop:
    clb         t0, 0(ca1)       # Load byte with capability bounds check
    beq         t0, zero, strcpy_end
    csb         t0, 0(ca0)       # Store byte with capability bounds check
    cincoffset  ca1, ca1, 1      # Increment source (bounds preserved)
    cincoffset  ca0, ca0, 1      # Increment destination (bounds preserved)
    j           cheri_strcpy_loop
strcpy_end:
    csb         zero, 0(ca0)     # Store null terminator (bounds checked)
```

#### Detailed Analysis:
**Critical Protection Points:**

**Bounds-Checked Load (`clb t0, 0(ca1)`):**
- **Hardware Check**: Verify `ca1.offset < ca1.base + ca1.length`
- **Tag Validation**: Ensure `ca1.tag == 1` (valid capability)
- **Permission Check**: Verify `ca1.permissions & Load`
- **Exception**: Capability bounds violation if check fails

**Bounds-Checked Store (`csb t0, 0(ca0)`):**
- **Hardware Check**: Verify `ca0.offset < ca0.base + ca0.length`
- **Critical Protection**: When `ca0.offset >= 8`, hardware triggers exception
- **Buffer Overflow Prevention**: Impossible to write beyond 8-byte buffer
- **Stack Protection**: Cannot corrupt adjacent stack variables

**Capability Arithmetic (`cincoffset ca0, ca0, 1`):**
- **Bounds Preservation**: Original capability bounds maintained
- **Monotonic Property**: Cannot expand bounds, only maintain or reduce
- **Overflow Detection**: Hardware detects when increment would exceed bounds

### Protected Function Epilogue
```assembly
    clc         cra, 48(csp)     # Step 9: Restore return capability
    clc         cs0, 32(csp)     # Step 10: Restore frame capability
    cincoffset  csp, csp, 64     # Step 11: Deallocate stack frame
    cret                         # Step 12: Return via capability
```

#### Security Analysis:
**Return Capability Protection:**
- **Sealed Capability**: Return capability cannot be forged
- **Authentication**: Hardware validates capability integrity
- **Control Flow Integrity**: Impossible to redirect to arbitrary code

## Memory Layout Comparison

### Standard RISC-V Stack Layout
```
Stack Frame (32 bytes):
+24: [return address]  ← 64-bit, vulnerable to corruption
+16: [saved s0]        ← 64-bit frame pointer
+ 8: [unused]
+ 0: [buffer start]    ← 8 bytes allocated, no bounds info
- 8: [buffer end]      
-16: [local var area]  ← Can be corrupted by buffer overflow
```

### CHERI Capability Stack Layout
```
Capability Stack Frame (64 bytes):
+48: [return capability]  ← 128-bit sealed capability
+32: [saved cs0]         ← 128-bit capability
+16: [unused]
+ 0: [buffer capability] ← Bounded to exactly 8 bytes
-16: [local cap area]    ← Protected by capability bounds
```

## Vulnerability Comparison

### Standard RISC-V Vulnerabilities
1. **Buffer Overflow**: No bounds checking allows unlimited writes
2. **Stack Corruption**: Can overwrite return address and frame data
3. **Control Flow Hijack**: Corrupted return address enables code execution
4. **Information Disclosure**: Out-of-bounds reads leak memory contents

### CHERI Protection Mechanisms
1. **Spatial Safety**: Hardware bounds checking prevents buffer overflow
2. **Temporal Safety**: Capability invalidation prevents use-after-free
3. **Control Flow Integrity**: Sealed return capabilities prevent hijacking
4. **Fine-grained Protection**: Object-level bounds vs page-level protection

## Performance Analysis

### Instruction Count Comparison
- **Standard RISC-V**: ~15 instructions for buffer operations
- **CHERI**: ~18 instructions (20% increase)
- **Overhead Source**: Additional capability instructions and bounds checking

### Memory Overhead
- **Standard RISC-V**: 8 bytes per pointer
- **CHERI**: 16 bytes per capability (100% increase)
- **Trade-off**: 2x memory usage for comprehensive security

## Conclusion

CHERI's capability-based protection provides comprehensive memory safety at the hardware level, preventing entire classes of vulnerabilities that are possible in Standard RISC-V. While there is a performance cost, the security benefits are substantial and provide deterministic protection rather than probabilistic software mitigations.
