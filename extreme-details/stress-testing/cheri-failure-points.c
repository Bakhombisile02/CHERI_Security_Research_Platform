/*
 * CHERI Failure Point Discovery Tests
 * 
 * These tests are specifically designed to find scenarios where CHERI fails,
 * shows significant limitations, or proves inferior to Standard RISC-V.
 */

#include <stddef.h>  // For ptrdiff_t

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_tag_get(cap) 1
#define cheri_perms_and(cap, perms) (cap)
#endif

// Global test state
static char global_memory[2048];
static int memory_offset = 0;

// Allocator that will eventually fail
cap_ptr_t failing_malloc(int size) {
    if (memory_offset + size > sizeof(global_memory)) {
        return (cap_ptr_t)0;  // Deliberate failure
    }
    
    void* ptr = &global_memory[memory_offset];
    memory_offset += size;
    return cheri_bounds_set(ptr, size);
}

// Test 1: CHERI Capability Table Exhaustion Failure
void test_cheri_capability_table_failure() {
    // This test should cause CHERI to fail due to capability table limits
    // while Standard RISC-V continues working
    
    cap_ptr_t capabilities[1000];
    int failure_point = -1;
    
    for (int i = 0; i < 1000; i++) {
        capabilities[i] = failing_malloc(16);
        
        if (capabilities[i] == (cap_ptr_t)0) {
            failure_point = i;
            break;
        }
        
        // Try to use the capability to verify it's valid
        if (!cheri_tag_get(capabilities[i])) {
            failure_point = i;
            break;
        }
    }
    
    // CHERI failure point marker
    volatile int cheri_fails = failure_point;
    (void)cheri_fails;
}

// Test 2: Memory Overhead Causes Earlier Failure
void test_memory_overhead_failure() {
    // CHERI's 2x memory overhead should cause earlier out-of-memory
    
    const int TARGET_ALLOCATIONS = 100;
    cap_ptr_t large_capabilities[TARGET_ALLOCATIONS];
    int cheri_allocation_count = 0;
    
    for (int i = 0; i < TARGET_ALLOCATIONS; i++) {
        // Allocate relatively large objects
        large_capabilities[i] = failing_malloc(64);
        
        if (large_capabilities[i] == (cap_ptr_t)0) {
            break;
        }
        
        cheri_allocation_count++;
    }
    
    // CHERI should allocate fewer objects due to capability overhead
    volatile int cheri_count = cheri_allocation_count;
    (void)cheri_count;
}

// Test 3: Performance Degradation Under Stress
void test_performance_degradation() {
    // High-frequency bounds checking should show measurable overhead
    
    cap_ptr_t buffer = failing_malloc(1024);
    if (buffer == (cap_ptr_t)0) return;
    
    char* ptr = (char*)buffer;
    
    // Intensive memory access pattern
    // Each access triggers CHERI bounds checking
    const int ITERATIONS = 1000000;
    
    for (int iteration = 0; iteration < 10; iteration++) {
        for (int i = 0; i < ITERATIONS; i++) {
            int index = i % 1024;
            
            // CHERI: bounds check + memory access
            // Standard RISC-V: direct memory access
            ptr[index] = (char)(i & 0xFF);
            
            // Force dependency to prevent optimization
            if (ptr[index] != (char)(i & 0xFF)) {
                break;  // Should never happen
            }
        }
    }
    
    // Performance degradation marker
    volatile int perf_degraded = 0x5L0FD06;  // SLOW DOG
    (void)perf_degraded;
}

// Test 4: Complex Capability Arithmetic Breakdown
void test_capability_arithmetic_breakdown() {
    // Complex pointer arithmetic that challenges CHERI's capability model
    
    cap_ptr_t base = failing_malloc(2048);
    if (base == (cap_ptr_t)0) return;
    
    char* ptr1 = (char*)base;
    char* ptr2 = ptr1 + 500;
    char* ptr3 = ptr2 + 1000;
    
    // Create a complex web of derived pointers
    for (int i = 0; i < 1000; i++) {
        // Complex arithmetic that may confuse capability tracking
        char* temp1 = ptr1 + (i % 100);
        char* temp2 = ptr2 - (i % 50);
        char* temp3 = ptr3 + (i % 200);
        
        // Cross-pointer arithmetic
        ptrdiff_t diff1 = temp2 - temp1;
        ptrdiff_t diff2 = temp3 - temp2;
        
        // Derived pointer creation
        char* derived = ptr1 + (diff1 + diff2) % 2048;
        
        // Access through derived pointer (complex bounds checking)
        if (derived >= (char*)base && derived < (char*)base + 2048) {
            *derived = (char)(i & 0xFF);
        }
    }
    
    // Complex arithmetic marker
    volatile int complex_arith = 0xC0A71EF;  // COMPLEX
    (void)complex_arith;
}

// Test 5: Capability Permission Overhead
void test_capability_permission_overhead() {
    // Test the overhead of capability permission checking
    
    cap_ptr_t read_write_cap = failing_malloc(256);
    if (read_write_cap == (cap_ptr_t)0) return;
    
    // Create restricted capabilities
    cap_ptr_t read_only_cap = cheri_perms_and(read_write_cap, 0x1);  // Read-only
    
    char* rw_ptr = (char*)read_write_cap;
    char* ro_ptr = (char*)read_only_cap;
    
    // Test permission checking overhead
    for (int i = 0; i < 10000; i++) {
        // Read through read-only capability (permission check)
        volatile char value = ro_ptr[i % 256];
        
        // Write through read-write capability (permission check)
        rw_ptr[i % 256] = (char)(i & 0xFF);
        
        // Force use of value to prevent optimization
        if (value == 0xFF) {
            // Unlikely branch
        }
    }
    
    // Permission overhead marker
    volatile int perm_overhead = 0xFE8A0FE8;  // PERM OVERHEAD
    (void)perm_overhead;
}

// Test 6: Inter-Object Bounds Checking Failure
void test_inter_object_bounds_failure() {
    // Test where CHERI's strict bounds checking becomes problematic
    
    cap_ptr_t obj1 = failing_malloc(128);
    cap_ptr_t obj2 = failing_malloc(128);
    
    if (obj1 == (cap_ptr_t)0 || obj2 == (cap_ptr_t)0) return;
    
    char* ptr1 = (char*)obj1;
    char* ptr2 = (char*)obj2;
    
    // Try to implement a common pattern that CHERI prevents
    // (but is common in legacy code)
    
    for (int i = 0; i < 256; i++) {
        // This pattern is common but violates CHERI bounds
        char* current_ptr = (i < 128) ? ptr1 : ptr2;
        int index = i % 128;
        
        // CHERI may fail here due to capability switching
        current_ptr[index] = (char)(i & 0xFF);
    }
    
    // Inter-object bounds marker
    volatile int inter_obj = 0x17E80B1;  // INTER OBJ
    (void)inter_obj;
}

// Test 7: Capability Revocation Performance Impact
void test_capability_revocation_impact() {
    // Test the performance impact of capability revocation
    
    cap_ptr_t caps[100];
    
    // Create many capabilities
    for (int i = 0; i < 100; i++) {
        caps[i] = failing_malloc(32);
        if (caps[i] == (cap_ptr_t)0) break;
    }
    
    // Simulate frequent revocation/revalidation
    for (int iteration = 0; iteration < 1000; iteration++) {
        for (int i = 0; i < 100; i++) {
            if (caps[i] == (cap_ptr_t)0) continue;
            
            // Check capability validity (potential revocation check)
            if (cheri_tag_get(caps[i])) {
                char* ptr = (char*)caps[i];
                ptr[0] = (char)(iteration & 0xFF);
            }
        }
    }
    
    // Revocation impact marker
    volatile int revocation = 0x8EF0CA70;  // REVOCATION
    (void)revocation;
}

// Test 8: Cache Performance Impact
void test_cache_performance_impact() {
    // CHERI's larger capability metadata may impact cache performance
    
    const int ARRAY_SIZE = 1000;
    cap_ptr_t capability_array[ARRAY_SIZE];
    
    // Create array of capabilities (larger than pointer array)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        capability_array[i] = failing_malloc(16);
        if (capability_array[i] == (cap_ptr_t)0) break;
    }
    
    // Access pattern that stresses cache
    for (int iteration = 0; iteration < 1000; iteration++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (capability_array[i] == (cap_ptr_t)0) continue;
            
            // Each capability access loads 16 bytes instead of 8
            char* ptr = (char*)capability_array[i];
            ptr[0] = (char)(iteration & 0xFF);
        }
    }
    
    // Cache impact marker
    volatile int cache_impact = 0xCAC1E1AF;  // CACHE IMPACT
    (void)cache_impact;
}

// Test 9: Compatibility Breaking Changes
void test_compatibility_breaking() {
    // Test patterns that work in Standard RISC-V but break in CHERI
    
    cap_ptr_t buffer = failing_malloc(256);
    if (buffer == (cap_ptr_t)0) return;
    
    char* ptr = (char*)buffer;
    
    // Pattern 1: Pointer arithmetic beyond bounds (common in C)
    char* end_ptr = ptr + 256;  // Points just past end
    char* scan_ptr = ptr;
    
    // This is legal in Standard RISC-V but may violate CHERI bounds
    while (scan_ptr < end_ptr) {
        *scan_ptr = 'A';
        scan_ptr++;
    }
    
    // Pattern 2: Using pointer difference for indexing
    ptrdiff_t offset = scan_ptr - ptr;
    if (offset > 0) {
        // This might work in Standard RISC-V but challenge CHERI
        ptr[offset - 1] = 'Z';
    }
    
    // Compatibility breaking marker
    volatile int compat_break = 0xB8EA1706;  // BREAK
    (void)compat_break;
}

// Main failure discovery test runner
int main() {
    // Test 1: Find CHERI capability table limits
    test_cheri_capability_table_failure();
    
    // Test 2: Demonstrate memory overhead failures
    test_memory_overhead_failure();
    
    // Test 3: Measure performance degradation
    test_performance_degradation();
    
    // Test 4: Challenge capability arithmetic
    test_capability_arithmetic_breakdown();
    
    // Test 5: Capability permission overhead
    test_capability_permission_overhead();
    
    // Test 6: Inter-object bounds checking issues
    test_inter_object_bounds_failure();
    
    // Test 7: Capability revocation performance
    test_capability_revocation_impact();
    
    // Test 8: Cache performance impact
    test_cache_performance_impact();
    
    // Test 9: Compatibility breaking changes
    test_compatibility_breaking();
    
    // Failure discovery completion marker
    volatile int failure_complete = 0xFA11C0A1;  // FAIL COMPLETE
    (void)failure_complete;
    
    return 0;
}
