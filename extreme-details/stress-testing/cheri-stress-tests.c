/*
 * CHERI Stress Test Suite - Capability Table Exhaustion
 * 
 * This test attempts to exhaust CHERI's capability table to find system limits.
 * Tests where CHERI may fail or show significant performance degradation.
 */

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_tag_get(cap) 1
#define cheri_tag_clear(cap) do { *(cap) = 0; } while(0)
#endif

// Test parameters
#define MAX_CAPABILITIES 100000  // Attempt to create many capabilities
#define LARGE_OBJECT_SIZE (1024 * 1024)  // 1MB objects

// Global arrays for stress testing
static cap_ptr_t capability_array[MAX_CAPABILITIES];
static char memory_pool[1024 * 1024];  // 1MB pool
static int pool_offset = 0;

// Simple allocator that creates bounded capabilities
cap_ptr_t stress_malloc(int size) {
    if (pool_offset + size > sizeof(memory_pool)) {
        return (cap_ptr_t)0;  // Out of memory
    }
    
    void* ptr = &memory_pool[pool_offset];
    pool_offset += size;
    
    // Create bounded capability - this consumes capability table entries
    return cheri_bounds_set(ptr, size);
}

// Test 1: Capability Table Exhaustion
void test_capability_table_exhaustion() {
    int successful_allocations = 0;
    int failed_allocations = 0;
    
    // Attempt to create maximum number of capabilities
    for (int i = 0; i < MAX_CAPABILITIES; i++) {
        capability_array[i] = stress_malloc(64);  // Small allocations
        
        if (capability_array[i] != (cap_ptr_t)0) {
            successful_allocations++;
        } else {
            failed_allocations++;
            break;  // Stop on first failure
        }
    }
    
    // Results will show CHERI's capability table limits
    volatile int result_success = successful_allocations;
    volatile int result_failed = failed_allocations;
    (void)result_success; (void)result_failed;
}

// Test 2: Memory Overhead Pressure
void test_memory_overhead_pressure() {
    // CHERI: 128-bit capabilities vs 64-bit pointers
    // This test shows the 2x memory overhead impact
    
    cap_ptr_t *capability_pointers[10000];
    
    for (int i = 0; i < 10000; i++) {
        // Each pointer in CHERI takes 16 bytes vs 8 bytes in Standard RISC-V
        capability_pointers[i] = stress_malloc(sizeof(cap_ptr_t));
        
        if (capability_pointers[i] == (cap_ptr_t)0) {
            // CHERI will run out of memory faster due to overhead
            break;
        }
    }
    
    // Memory pressure marker
    volatile int memory_pressure = 0xAE40ADE5;  // MEMORY PRESSURE
    (void)memory_pressure;
}

// Test 3: Performance-Critical Memory Access Pattern
void test_performance_critical_access() {
    const int ACCESS_COUNT = 1000000;  // 1 million accesses
    char* buffer = (char*)stress_malloc(4096);
    
    if (buffer == (cap_ptr_t)0) return;
    
    // Tight loop with frequent memory access
    // CHERI will perform bounds checking on every access
    for (int iteration = 0; iteration < 1000; iteration++) {
        for (int i = 0; i < ACCESS_COUNT; i++) {
            int index = i % 4096;
            
            // This access will trigger CHERI bounds checking
            // Standard RISC-V: simple pointer dereference
            // CHERI: capability bounds validation + memory access
            buffer[index] = (char)(i & 0xFF);
        }
    }
    
    // Performance overhead marker
    volatile int perf_test = 0xFE8F7E57;  // PERF TEST
    (void)perf_test;
}

// Test 4: Complex Pointer Arithmetic Stress
void test_complex_pointer_arithmetic() {
    char* base_buffer = (char*)stress_malloc(8192);
    if (base_buffer == (cap_ptr_t)0) return;
    
    // Complex multi-level pointer manipulation
    char* ptr1 = base_buffer;
    char* ptr2 = ptr1 + 1000;
    char* ptr3 = ptr2 + 2000;
    char* ptr4 = ptr3 + 3000;
    
    // Chain of pointer arithmetic operations
    // CHERI must track bounds through each operation
    for (int i = 0; i < 10000; i++) {
        ptr1 = ptr1 + (i % 100);
        ptr2 = ptr1 + 500;
        ptr3 = ptr2 + 1000;
        ptr4 = ptr3 + 1500;
        
        // Access through each pointer (bounds checking on each)
        if (ptr4 < (char*)base_buffer + 8192) {  // Bounds check
            *ptr4 = (char)(i & 0xFF);
        }
        
        // Reset pointers periodically
        if (i % 1000 == 0) {
            ptr1 = base_buffer;
        }
    }
    
    // Pointer arithmetic stress marker
    volatile int ptr_arith = 0xA817EA7C;  // ARITHMETIC
    (void)ptr_arith;
}

// Test 5: Deep Call Stack with Capability Frames
void recursive_capability_stress(int depth, cap_ptr_t data) {
    if (depth <= 0) return;
    
    // Each recursive call creates new capability frame
    char local_buffer[256];
    cap_ptr_t local_cap = cheri_bounds_set(local_buffer, sizeof(local_buffer));
    
    // Access through capability (bounds checking)
    local_buffer[0] = 'A';
    local_buffer[255] = 'Z';
    
    // Recursive call with capability passing
    recursive_capability_stress(depth - 1, local_cap);
    
    // Use the passed capability (more bounds checking)
    if (cheri_tag_get(data)) {
        // Capability is valid, use it
        char* data_ptr = (char*)data;
        data_ptr[0] = 'X';
    }
}

void test_deep_call_stack_stress() {
    char initial_buffer[1024];
    cap_ptr_t initial_cap = cheri_bounds_set(initial_buffer, sizeof(initial_buffer));
    
    // Deep recursion with capability management
    recursive_capability_stress(1000, initial_cap);  // 1000 levels deep
    
    // Deep recursion marker
    volatile int deep_recursion = 0x8EE56EC5;  // DEEP RECURSION
    (void)deep_recursion;
}

// Test 6: Pathological Case - Tiny Objects with Full Capabilities
void test_capability_overhead_pathological() {
    // Create many tiny objects, each requiring full capability metadata
    // Shows worst-case overhead: 16-byte capability for 1-byte object
    
    for (int i = 0; i < 10000; i++) {
        cap_ptr_t tiny_cap = stress_malloc(1);  // 1 byte allocation
        
        if (tiny_cap == (cap_ptr_t)0) break;
        
        // Use the capability (triggering bounds checking for tiny object)
        char* tiny_ptr = (char*)tiny_cap;
        *tiny_ptr = (char)(i & 0xFF);
    }
    
    // Pathological overhead marker
    volatile int pathological = 0xFA71010C;  // PATHOLOGICAL
    (void)pathological;
}

// Test 7: Simulation of Real-World Application - String Processing
void test_string_processing_workload() {
    const int STRING_COUNT = 10000;
    const int MAX_STRING_LENGTH = 256;
    
    cap_ptr_t strings[STRING_COUNT];
    
    // Allocate many strings (typical application pattern)
    for (int i = 0; i < STRING_COUNT; i++) {
        int length = (i % MAX_STRING_LENGTH) + 1;
        strings[i] = stress_malloc(length);
        
        if (strings[i] == (cap_ptr_t)0) break;
        
        // Initialize string (bounds checking on each character)
        char* str = (char*)strings[i];
        for (int j = 0; j < length - 1; j++) {
            str[j] = 'A' + (j % 26);
        }
        str[length - 1] = '\0';
    }
    
    // Process strings (intensive capability use)
    for (int i = 0; i < STRING_COUNT; i++) {
        if (strings[i] == (cap_ptr_t)0) break;
        
        char* str = (char*)strings[i];
        
        // String length calculation (bounds checking on each access)
        int len = 0;
        while (str[len] != '\0' && len < MAX_STRING_LENGTH) {
            len++;
        }
        
        // String reversal (more bounds checking)
        for (int j = 0; j < len / 2; j++) {
            char temp = str[j];
            str[j] = str[len - 1 - j];
            str[len - 1 - j] = temp;
        }
    }
    
    // String workload marker
    volatile int string_workload = 0x57816570;  // STRING WORKLOAD
    (void)string_workload;
}

// Main stress test runner
int main() {
    // Test 1: Find CHERI's capability table limits
    test_capability_table_exhaustion();
    
    // Test 2: Demonstrate memory overhead impact
    test_memory_overhead_pressure();
    
    // Test 3: Performance-critical access patterns
    test_performance_critical_access();
    
    // Test 4: Complex pointer arithmetic stress
    test_complex_pointer_arithmetic();
    
    // Test 5: Deep call stack with capabilities
    test_deep_call_stack_stress();
    
    // Test 6: Pathological capability overhead case
    test_capability_overhead_pathological();
    
    // Test 7: Real-world application simulation
    test_string_processing_workload();
    
    // Completion marker
    volatile int stress_complete = 0x57BF55C0;  // STRESS COMPLETE
    (void)stress_complete;
    
    return 0;
}
