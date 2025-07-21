# Edge Cases and Boundary Condition Testing

This directory contains comprehensive testing of edge cases, boundary conditions, and corner case scenarios for both Standard RISC-V and Authentic CHERI implementations.

## Testing Philosophy

### Edge Case Categories
1. **Spatial Boundary Testing**: Memory bounds edge conditions
2. **Temporal Safety Edge Cases**: Timing-related vulnerability windows
3. **Capability System Limits**: CHERI-specific boundary conditions
4. **Performance Stress Testing**: Resource exhaustion scenarios
5. **Theoretical Attack Vectors**: Research-based vulnerability testing

## Directory Structure

```
edge-cases/
├── spatial-boundaries/       # Memory bounds testing
│   ├── off-by-one/          # Classic off-by-one errors
│   ├── zero-length-objects/ # Empty object handling
│   ├── maximum-size-objects/ # Large object allocation
│   └── alignment-issues/    # Memory alignment edge cases
├── temporal-safety/         # Time-based vulnerabilities
│   ├── race-conditions/     # Concurrent access patterns
│   ├── delayed-free/        # Timing-dependent use-after-free
│   └── capability-lifetime/ # CHERI capability expiration
├── cheri-specific/          # CHERI architecture edge cases
│   ├── capability-arithmetic/ # Capability calculation limits
│   ├── permission-boundaries/ # Permission bit edge cases
│   ├── tag-bit-manipulation/ # Tag validity edge cases
│   └── compartment-crossing/ # Inter-compartment boundary tests
├── stress-testing/          # Resource exhaustion tests
│   ├── memory-exhaustion/   # Low memory conditions
│   ├── capability-exhaustion/ # CHERI capability table limits
│   └── stack-depth-limits/  # Deep recursion testing
└── theoretical-attacks/     # Research-based attack scenarios
    ├── novel-exploits/      # Emerging attack techniques
    ├── rop-gadget-analysis/ # Return-oriented programming attempts
    └── side-channel-tests/  # Timing and cache-based attacks
```

## Spatial Boundary Edge Cases

### Off-by-One Error Testing

#### Test Case: Array Access Boundary
```c
// Edge case: Access at exact array boundary
int array[10];
int index = 10;  // Legal range: 0-9, testing index 10

// Standard RISC-V expectation: Memory corruption
// CHERI expectation: Capability bounds violation
int value = array[index];
```

#### Expected Behaviors

**Standard RISC-V:**
```assembly
# Calculate address: base + (index * 4)
slli    t0, a1, 2        # index * 4
add     t0, a0, t0       # base + offset
lw      t1, 0(t0)        # Load from potentially invalid address

# Result: May succeed, may segfault, may corrupt memory
# Security: Undefined behavior, potential exploitation
```

**Authentic CHERI:**
```assembly
# Capability-based array access
slli    t0, a1, 2        # index * 4
cincoffset ca0, ca0, t0  # Add offset to capability
clw     t1, 0(ca0)       # Load with capability bounds check

# Result: Capability bounds exception if index >= 10
# Security: Deterministic protection, no memory corruption
```

### Zero-Length Object Testing

#### Test Case: Empty Buffer Operations
```c
// Edge case: Operations on zero-length buffer
char *buffer = malloc(0);  // Platform-specific behavior
strcpy(buffer, "data");    // Writing to zero-length allocation

// Standard RISC-V: Undefined behavior, likely corruption
// CHERI: Capability with zero length, immediate bounds violation
```

#### CHERI Protection Analysis
```c
// CHERI malloc(0) behavior
void *cheri_malloc(size_t size) {
    if (size == 0) {
        // Return capability with zero length
        void *ptr = allocate_memory(1);  // Minimal allocation
        return csetbounds(ptr, 0);       // Zero-length capability
    }
    // Normal allocation...
}

// Any access to zero-length capability triggers exception
```

## Temporal Safety Edge Cases

### Delayed Use-After-Free

#### Test Case: Timing Window Exploitation
```c
// Edge case: Use-after-free with timing dependency
void *ptr = malloc(1024);
free(ptr);

// Timing window: ptr still contains "valid" address
// but memory may be reallocated to different purpose
sleep_microseconds(random_delay());  // Variable timing

// Attempt access after random delay
*((int*)ptr) = 0xdeadbeef;

// Standard RISC-V: May succeed, corrupt unrelated data
// CHERI: Capability invalidated at free(), guaranteed protection
```

#### CHERI Capability Lifecycle
```assembly
# CHERI free() implementation
ccall   free               # Call CHERI-aware free
# Inside free():
cstore  cnull, 0(ptr_cap)  # Clear capability to prevent reuse
# Capability is now invalid regardless of timing
```

### Race Condition Testing

#### Test Case: Concurrent Capability Manipulation
```c
// Edge case: Concurrent access to capability metadata
void *shared_ptr;

// Thread 1: Modifies capability bounds
void thread1() {
    shared_ptr = csetbounds(shared_ptr, new_size);
}

// Thread 2: Accesses through capability
void thread2() {
    *((int*)shared_ptr) = value;
}

// Standard RISC-V: Race on pointer value, undefined behavior
// CHERI: Atomic capability operations prevent races
```

## CHERI-Specific Edge Cases

### Capability Arithmetic Limits

#### Test Case: Maximum Offset Calculation
```c
// Edge case: Capability offset at maximum representable value
void *base = malloc(SIZE_MAX);
void *ptr = base;

// Increment to maximum possible offset
for (size_t i = 0; i < SIZE_MAX; i++) {
    ptr = (char*)ptr + 1;  // May overflow capability bounds
}

// CHERI consideration: Capability arithmetic overflow behavior
```

#### CHERI Overflow Protection
```assembly
# CHERI capability arithmetic with overflow detection
cincoffset  ca0, ca0, t0    # Add offset to capability
# Hardware checks:
# 1. Offset + current_offset doesn't overflow
# 2. Result stays within original capability bounds
# 3. Tag bit preserved only if bounds maintained
```

### Permission Boundary Testing

#### Test Case: Granular Permission Enforcement
```c
// Edge case: Capability with specific permission combinations
void *read_only_cap = get_read_only_capability(buffer);
void *write_only_cap = get_write_only_capability(buffer);

// Test permission boundaries
int value = *((int*)read_only_cap);        // Should succeed
*((int*)write_only_cap) = 42;              // Should succeed
*((int*)read_only_cap) = 42;               // Should fail
int value2 = *((int*)write_only_cap);      // Should fail

// CHERI: Granular permission enforcement at instruction level
```

## Stress Testing Scenarios

### Memory Exhaustion Edge Cases

#### Test Case: Low Memory with Capability Overhead
```c
// Edge case: Memory exhaustion in CHERI vs Standard RISC-V
void stress_test_allocation() {
    void **ptrs = NULL;
    size_t count = 0;
    
    // Allocate until failure
    while (1) {
        void *ptr = malloc(1024);
        if (!ptr) break;
        
        // Reallocate pointer array
        ptrs = realloc(ptrs, (count + 1) * sizeof(void*));
        ptrs[count++] = ptr;
    }
    
    // Compare allocation counts:
    // Standard RISC-V: ~N allocations
    // CHERI: ~N/2 allocations (due to capability overhead)
}
```

### Stack Depth Limits

#### Test Case: Deep Recursion with Capability Stack
```c
// Edge case: Stack exhaustion in capability vs traditional stack
int deep_recursion(int depth) {
    char local_buffer[1024];  // Stack allocation
    
    if (depth <= 0) return 0;
    return deep_recursion(depth - 1) + 1;
}

// Standard RISC-V: Stack overflow corrupts memory
// CHERI: Capability stack bounds prevent corruption
```

## Theoretical Attack Scenarios

### Novel Exploitation Attempts

#### Test Case: Return-Oriented Programming Against CHERI
```c
// Edge case: ROP gadget construction in CHERI environment
void vulnerable_function(char *input) {
    char buffer[64];
    strcpy(buffer, input);  // Buffer overflow
    
    // Traditional ROP: Overwrite return address with gadget chain
    // CHERI consideration: Return capability vs return address
}

// CHERI Protection Analysis:
// 1. Return address is capability, not raw address
// 2. ROP gadgets need valid executable capabilities
// 3. Capability permissions prevent arbitrary code execution
```

#### CHERI ROP Mitigation
```assembly
# Traditional ROP exploitation
# Stack layout: [buffer][saved_fp][return_addr]
# Attack: Overwrite return_addr with gadget address

# CHERI stack layout: [buffer][saved_capability][return_capability]
# Attack complexity: Must forge valid executable capability
# Protection: Hardware validates capability authenticity
```

### Side-Channel Attack Testing

#### Test Case: Timing Analysis of Capability Operations
```c
// Edge case: Timing differences in capability vs pointer operations
void timing_test() {
    clock_t start, end;
    
    // Standard pointer operations
    start = clock();
    for (int i = 0; i < 1000000; i++) {
        void *ptr = array + i;
        volatile int val = *((int*)ptr);
    }
    end = clock();
    printf("Standard timing: %ld\n", end - start);
    
    // CHERI capability operations
    start = clock();
    for (int i = 0; i < 1000000; i++) {
        void *cap = cincoffset(array_cap, i * sizeof(int));
        volatile int val = *((int*)cap);
    }
    end = clock();
    printf("CHERI timing: %ld\n", end - start);
}

// Analysis: Does capability bounds checking introduce timing variations?
// Security: Could timing differences leak bounds information?
```

## Edge Case Documentation Standards

### Test Case Format
1. **Scenario Description**: Clear explanation of edge condition
2. **Expected Behavior**: Predicted outcome for each architecture
3. **Security Implications**: Vulnerability vs protection analysis
4. **Test Implementation**: Concrete code to reproduce scenario
5. **Result Analysis**: Actual behavior comparison

### Boundary Condition Categories
- **Exact Boundaries**: Testing precise limits (array[size-1] vs array[size])
- **Overflow Conditions**: Testing beyond limits (integer overflow, capability overflow)
- **Underflow Conditions**: Testing below limits (negative indices, invalid offsets)
- **Zero Conditions**: Testing empty/null cases (zero-length, null pointers)
- **Maximum Conditions**: Testing resource limits (memory exhaustion, stack limits)

This comprehensive edge case testing framework ensures thorough evaluation of both architectures under extreme conditions, revealing subtle security differences and implementation robustness.
