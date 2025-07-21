# CHERI Capability Deep Dive - Complete Technical Analysis

## Overview
This document provides a comprehensive technical analysis of CHERI's 128-bit capability structure, protection mechanisms, and hardware implementation details.

## CHERI Capability Structure (128-bit Compressed Format)

### Bit-Level Layout
```
CHERI Capability (128 bits total):

┌─────────────────────────────────────────────────────────────────┐
│                    Metadata (64 bits)                          │
├─────────────────────────────────────────────────────────────────┤
│                Address/Offset (64 bits)                        │
└─────────────────────────────────────────────────────────────────┘

Detailed Metadata Structure:
[127:126] Capability Type (2 bits) - Sealed/Unsealed/Sentry
[125:109] Compressed Bounds (17 bits) - Base/Top encoding
[108:96]  Permissions (13 bits) - Read/Write/Execute/etc.
[95:64]   Object Type (32 bits) - Software-defined type

Address Structure:
[63:0]    Virtual Address/Offset - Pointer value within capability bounds
```

### Capability Types
```c
typedef enum {
    CAP_TYPE_UNSEALED = 0,    // Normal memory capability
    CAP_TYPE_SEALED = 1,      // Sealed capability (cannot be used for memory access)
    CAP_TYPE_SENTRY = 2,      // Sentry capability (for function calls)
    CAP_TYPE_RESERVED = 3     // Reserved for future use
} cheri_cap_type_t;
```

### Permission Bits Detailed Analysis
```
CHERI Permissions (13 bits):
┌─ Bit ─┬─ Name ──────────────┬─ Description ──────────────────────────────┐
│  12   │ Global              │ Can be stored to memory                    │
│  11   │ Execute             │ Can be used for instruction fetch          │
│  10   │ Load                │ Can be used for data loads                 │
│   9   │ Store               │ Can be used for data stores                │
│   8   │ LoadCap             │ Can load capabilities from memory          │
│   7   │ StoreCap            │ Can store capabilities to memory           │
│   6   │ StoreLocal          │ Can store non-global capabilities          │
│   5   │ Seal                │ Can be used to seal capabilities           │
│   4   │ CInvoke             │ Can be used for compartment calls          │
│   3   │ Unseal              │ Can unseal capabilities                    │
│   2   │ Access_System_Regs  │ Can access system registers                │
│   1   │ User1               │ User-defined permission bit               │
│   0   │ User0               │ User-defined permission bit               │
└───────┴─────────────────────┴────────────────────────────────────────────┘

Permission Combinations:
- Read-Only Data:    Load | Global
- Read-Write Data:   Load | Store | Global  
- Executable Code:   Execute | Load | Global
- Full Data Access:  Load | Store | LoadCap | StoreCap | Global
```

## Compressed Bounds Representation

### Bounds Compression Algorithm
CHERI uses a compressed bounds representation to encode arbitrary memory regions within limited bits.

```c
// Conceptual bounds encoding structure
struct compressed_bounds {
    uint8_t exponent;     // Determines alignment and representable size
    uint16_t mantissa;    // Fine-grained bounds within aligned region
    bool internal_exponent; // Whether exponent is encoded separately
};

// Bounds calculation (simplified)
uint64_t calculate_bounds(struct compressed_bounds cb) {
    if (cb.exponent == 0) {
        // Precise bounds for small objects
        base = mantissa_base << 0;
        top = mantissa_top << 0;
    } else {
        // Aligned bounds for larger objects
        uint64_t alignment = 1ULL << cb.exponent;
        base = (mantissa_base << cb.exponent) & ~(alignment - 1);
        top = (mantissa_top << cb.exponent) | (alignment - 1);
    }
    return (top << 32) | base;
}
```

### Representable Bounds Examples

#### Small Objects (< 4KB) - Byte-Precise Bounds
```
Object Size: 10 bytes
Requested Bounds: [0x1000, 0x100a)
Representable: [0x1000, 0x100a) - Exact match
Exponent: 0
Mantissa: Encodes exact base and top
```

#### Medium Objects (4KB - 1MB) - Aligned Bounds
```
Object Size: 8KB
Requested Bounds: [0x10000, 0x12000)
Representable: [0x10000, 0x12000) - 16-byte aligned
Exponent: 4 (2^4 = 16-byte alignment)
Mantissa: Encodes aligned base and top
```

#### Large Objects (> 1MB) - Page-Aligned Bounds
```
Object Size: 2MB
Requested Bounds: [0x100000, 0x300000)
Representable: [0x100000, 0x300000) - 4KB aligned
Exponent: 12 (2^12 = 4KB alignment)
Mantissa: Encodes page-aligned bounds
```

### Bounds Rounding Behavior
When requested bounds don't align to representable values, CHERI rounds **outward** to maintain safety:

```c
// Example: Request 13-byte buffer
void *ptr = malloc(13);

// Standard pointer: Exactly 13 bytes accessible
// CHERI capability: May round to 16 bytes (next representable size)

// Safety guarantee: Never rounds inward (never less than requested)
// Security implication: Slight over-allocation but never under-protection
```

## Hardware Protection Mechanisms

### Spatial Memory Safety

#### Bounds Checking on Every Access
```assembly
# Standard RISC-V load (no protection)
ld      t0, 0(a0)           # Load 8 bytes from address in a0
                            # No bounds checking - may access arbitrary memory

# CHERI capability load (bounds checked)
clc     ct0, 0(ca0)         # Load capability from capability ca0
                            # Hardware automatically checks:
                            # 1. ca0.tag == 1 (valid capability)
                            # 2. ca0.offset >= ca0.base
                            # 3. ca0.offset + 16 <= ca0.base + ca0.length
                            # 4. ca0.permissions & LoadCap
```

#### Capability Arithmetic Protection
```assembly
# Pointer arithmetic (Standard RISC-V)
addi    a0, a0, 8           # a0 = a0 + 8 (unchecked arithmetic)
                            # Can create out-of-bounds pointer

# Capability arithmetic (CHERI)
cincoffset ca0, ca0, 8      # ca0.offset = ca0.offset + 8
                            # Hardware checks:
                            # 1. Result stays within original bounds
                            # 2. Capability remains valid
                            # 3. If bounds exceeded: clear tag bit
```

### Temporal Memory Safety

#### Capability Invalidation on Free
```c
// CHERI malloc/free implementation (conceptual)
void *cheri_malloc(size_t size) {
    void *mem = allocate_memory(size);
    // Create bounded capability
    void *cap = __builtin_cheri_bounds_set(mem, size);
    return cap;
}

void cheri_free(void *ptr) {
    // Standard deallocation
    deallocate_memory(ptr);
    
    // CHERI-specific: Invalidate all references to this capability
    // This is typically done by the allocator clearing tag bits
    // in all known references to this memory region
    invalidate_capability_references(ptr);
}
```

#### Use-After-Free Prevention
```assembly
# After cheri_free(), capability tag bit is cleared
clc     ct0, 0(ca0)         # Attempt to load from freed capability
                            # Hardware checks ca0.tag bit
                            # If tag == 0: Capability tag violation exception
                            # Program terminated, no memory corruption possible
```

## CHERI Instruction Set Extensions

### Capability Load/Store Instructions
```assembly
# Capability loads (bounds checked)
clb     rd, offset(cs)      # Load byte with capability bounds check
clh     rd, offset(cs)      # Load halfword with capability bounds check  
clw     rd, offset(cs)      # Load word with capability bounds check
cld     rd, offset(cs)      # Load doubleword with capability bounds check
clc     cd, offset(cs)      # Load capability with capability bounds check

# Capability stores (bounds checked)
csb     rs, offset(cd)      # Store byte with capability bounds check
csh     rs, offset(cd)      # Store halfword with capability bounds check
csw     rs, offset(cd)      # Store word with capability bounds check
csd     rs, offset(cd)      # Store doubleword with capability bounds check
csc     cs, offset(cd)      # Store capability with capability bounds check
```

### Capability Manipulation Instructions
```assembly
# Capability arithmetic
cincoffset cd, cs, rs       # Add integer offset to capability
csetoffset cd, cs, rs       # Set capability offset to integer value
csetaddr   cd, cs, rs       # Set capability address to integer value

# Bounds manipulation
csetbounds cd, cs, rs       # Set capability bounds (monotonic - can only narrow)
croundrepresentablelength rd, rs  # Round length to representable value
crepresentablealignmentmask rd, rs # Get alignment mask for length

# Capability inspection
cgetbase   rd, cs           # Get capability base address
cgetlen    rd, cs           # Get capability length
cgetoffset rd, cs           # Get capability offset
cgetaddr   rd, cs           # Get capability address (base + offset)
cgetperm   rd, cs           # Get capability permissions
cgettype   rd, cs           # Get capability type
ctag       rd, cs           # Get capability tag bit
```

### Capability Permission Manipulation
```assembly
# Permission modification
candperm   cd, cs, rs       # Clear permission bits (bitwise AND)
csetflags  cd, cs, rs       # Set user-defined flag bits

# Sealing and unsealing
cseal      cd, cs, ct       # Seal capability with type from ct
cunseal    cd, cs, ct       # Unseal capability with type from ct

# Special capabilities
cspecialr  cd, scr          # Read special capability register
cspecialw  scr, cs          # Write special capability register
```

## Memory Management with Capabilities

### Capability-Aware Malloc Implementation
```c
// Simplified CHERI malloc implementation
void *cheri_malloc(size_t size) {
    // Standard memory allocation
    void *raw_ptr = allocate_memory_pages(size);
    
    // Create capability with exact bounds
    void *bounded_cap = __builtin_cheri_bounds_set(raw_ptr, size);
    
    // Set appropriate permissions
    void *perm_cap = __builtin_cheri_perms_and(bounded_cap, 
                        CHERI_PERM_LOAD | CHERI_PERM_STORE | CHERI_PERM_GLOBAL);
    
    return perm_cap;
}

// Capability metadata tracking
struct cheri_alloc_metadata {
    void *capability;           // Original capability returned to user
    size_t requested_size;      // User-requested size
    size_t actual_size;         // Rounded size for bounds representability
    uint64_t allocation_id;     // Unique allocation identifier
    struct cheri_alloc_metadata *next;
};
```

### Stack Capability Management
```assembly
# CHERI function prologue - capability stack management
function_entry:
    # Adjust capability stack pointer (bounds preserved)
    cincoffset  csp, csp, -frame_size
    
    # Store return capability (sealed by hardware)
    csc         cra, return_offset(csp)
    
    # Store caller's frame capability
    csc         cfp, frame_offset(csp)
    
    # Create new frame capability with precise bounds
    cincoffset  cfp, csp, frame_size
    csetbounds  cfp, cfp, frame_size    # Bound frame to exact size
```

## Security Properties and Guarantees

### Spatial Safety Guarantees
1. **Bounds Checking**: Hardware enforces memory bounds on every access
2. **Pointer Arithmetic Safety**: Capability arithmetic preserves bounds
3. **Buffer Overflow Prevention**: Impossible to access beyond allocated bounds
4. **Array Bounds Protection**: Array accesses automatically bounds-checked

### Temporal Safety Guarantees
1. **Use-After-Free Prevention**: Freed capabilities automatically invalidated
2. **Double-Free Protection**: Invalid capabilities cannot be freed again
3. **Dangling Pointer Elimination**: No valid references to freed memory
4. **Lifetime Management**: Automatic capability invalidation on deallocation

### Control Flow Integrity
1. **Return Address Protection**: Return capabilities cannot be forged
2. **Function Pointer Safety**: Code capabilities prevent arbitrary execution
3. **Compartmentalization**: Sealed capabilities enable secure domains
4. **Call Gate Protection**: Sentry capabilities control domain transitions

## Performance Characteristics

### Memory Overhead Analysis
```c
// Memory usage comparison
struct example {
    char *ptr1;          // Standard: 8 bytes
    char *ptr2;          // Standard: 8 bytes
    int value;           // Standard: 4 bytes
};                       // Total: 20 bytes (+ padding)

struct cheri_example {
    char * __capability ptr1;  // CHERI: 16 bytes
    char * __capability ptr2;  // CHERI: 16 bytes  
    int value;                 // CHERI: 4 bytes
};                             // Total: 36 bytes (+ padding)

// Memory overhead: 80% increase for pointer-heavy structures
```

### Instruction Overhead Analysis
```assembly
# Standard RISC-V buffer access (3 instructions)
addi    a0, sp, offset      # Calculate buffer address
li      t0, value           # Load immediate value
sb      t0, 0(a0)           # Store to buffer

# CHERI buffer access (4 instructions + bounds checking)
cincoffset ca0, csp, offset # Calculate buffer capability
csetbounds ca0, ca0, size   # Set precise bounds
li         t0, value        # Load immediate value
csb        t0, 0(ca0)       # Store to buffer (bounds checked)

# Instruction overhead: ~33% increase + hardware bounds checking cost
```

### Hardware Implementation Costs
1. **Capability Registers**: 128-bit register file (vs 64-bit)
2. **Tag Memory**: Additional bit per 128-bit aligned memory location
3. **Bounds Checking Logic**: Hardware bounds comparison on every access
4. **Capability ALU**: Extended arithmetic units for capability operations

## Security vs Performance Trade-offs

### Benefits
- **Complete Spatial Safety**: Eliminates buffer overflows, array bounds violations
- **Complete Temporal Safety**: Eliminates use-after-free, double-free vulnerabilities
- **Fine-Grained Protection**: Object-level vs page-level protection granularity
- **Hardware Enforcement**: Deterministic protection, not probabilistic
- **Compartmentalization**: Capability-based isolation and privilege separation

### Costs
- **Memory Overhead**: 2x for pointers (64-bit → 128-bit capabilities)
- **Instruction Overhead**: ~20-40% increase in instruction count
- **Cache Pressure**: Larger data structures reduce cache efficiency
- **Hardware Complexity**: Additional silicon for capability processing

### Deployment Considerations
1. **Compatibility**: Requires CHERI-aware compilation and runtime
2. **Migration Path**: Hybrid mode allows gradual capability adoption
3. **Performance Tuning**: Careful capability usage in performance-critical code
4. **Memory Budget**: Account for 2x pointer memory usage in system design

## Conclusion

CHERI's capability-based protection provides comprehensive memory safety through hardware enforcement of spatial and temporal safety properties. While there are performance costs, the security benefits are substantial, eliminating entire classes of memory safety vulnerabilities that have plagued computing systems for decades.

The capability system represents a fundamental shift from address-based to capability-based computing, providing fine-grained, hardware-enforced protection that scales from individual objects to entire system compartmentalization.
