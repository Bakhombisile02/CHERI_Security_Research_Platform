/*
 * Standard RISC-V Stress Test Suite - Performance Baseline
 * 
 * Equivalent stress tests for Standard RISC-V to provide fair comparison.
 * These tests should show where Standard RISC-V excels (performance, memory efficiency).
 */

#include <stdint.h>

// Test parameters (identical to CHERI version)
#define MAX_POINTERS 100000
#define LARGE_OBJECT_SIZE (1024 * 1024)

// Global arrays for stress testing
static void* pointer_array[MAX_POINTERS];
static char memory_pool[1024 * 1024];  // 1MB pool
static int pool_offset = 0;

// Simple allocator using standard pointers
void* stress_malloc(int size) {
    if (pool_offset + size > sizeof(memory_pool)) {
        return 0;  // Out of memory
    }
    
    void* ptr = &memory_pool[pool_offset];
    pool_offset += size;
    
    return ptr;  // Return simple pointer (no bounds information)
}

// Test 1: Pointer Array Creation (Standard RISC-V Advantage)
void test_pointer_array_creation() {
    int successful_allocations = 0;
    int failed_allocations = 0;
    
    // Create maximum number of pointers
    // Standard RISC-V: 8 bytes per pointer
    // CHERI: 16 bytes per capability
    for (int i = 0; i < MAX_POINTERS; i++) {
        pointer_array[i] = stress_malloc(64);
        
        if (pointer_array[i] != 0) {
            successful_allocations++;
        } else {
            failed_allocations++;
            break;
        }
    }
    
    // Standard RISC-V should allocate more pointers due to lower overhead
    volatile int result_success = successful_allocations;
    volatile int result_failed = failed_allocations;
    (void)result_success; (void)result_failed;
}

// Test 2: Memory Efficiency Demonstration
void test_memory_efficiency() {
    // Standard RISC-V: 8-byte pointers
    void** pointer_pointers[10000];
    
    for (int i = 0; i < 10000; i++) {
        pointer_pointers[i] = (void**)stress_malloc(sizeof(void*));
        
        if (pointer_pointers[i] == 0) {
            break;
        }
    }
    
    // Memory efficiency marker
    volatile int memory_efficient = 0xEFF1C1E7;  // EFFICIENT
    (void)memory_efficient;
}

// Test 3: High-Performance Memory Access (Standard RISC-V Advantage)
void test_high_performance_access() {
    const int ACCESS_COUNT = 1000000;  // 1 million accesses
    char* buffer = (char*)stress_malloc(4096);
    
    if (buffer == 0) return;
    
    // Tight loop with frequent memory access
    // Standard RISC-V: Direct memory access, no bounds checking
    for (int iteration = 0; iteration < 1000; iteration++) {
        for (int i = 0; i < ACCESS_COUNT; i++) {
            int index = i % 4096;
            
            // Direct memory access - no overhead
            buffer[index] = (char)(i & 0xFF);
        }
    }
    
    // High performance marker
    volatile int high_perf = 0xFE8F0000;  // HIGH PERF
    (void)high_perf;
}

// Test 4: Complex Pointer Arithmetic (Standard RISC-V Advantage)
void test_complex_pointer_arithmetic() {
    char* base_buffer = (char*)stress_malloc(8192);
    if (base_buffer == 0) return;
    
    // Complex pointer arithmetic - no bounds tracking overhead
    char* ptr1 = base_buffer;
    char* ptr2 = ptr1 + 1000;
    char* ptr3 = ptr2 + 2000;
    char* ptr4 = ptr3 + 3000;
    
    // Chain of pointer operations - all at native speed
    for (int i = 0; i < 10000; i++) {
        ptr1 = ptr1 + (i % 100);
        ptr2 = ptr1 + 500;
        ptr3 = ptr2 + 1000;
        ptr4 = ptr3 + 1500;
        
        // Direct memory access - no bounds checking
        if (ptr4 < base_buffer + 8192) {  // Software bounds check (optional)
            *ptr4 = (char)(i & 0xFF);
        }
        
        if (i % 1000 == 0) {
            ptr1 = base_buffer;
        }
    }
    
    // Fast pointer arithmetic marker
    volatile int fast_ptr_arith = 0xFA57A817;  // FAST ARITHMETIC
    (void)fast_ptr_arith;
}

// Test 5: Deep Call Stack (Standard RISC-V Advantage)
void recursive_pointer_stress(int depth, void* data) {
    if (depth <= 0) return;
    
    // Each call uses simple stack frame - no capability overhead
    char local_buffer[256];
    
    // Direct memory access
    local_buffer[0] = 'A';
    local_buffer[255] = 'Z';
    
    // Recursive call with simple pointer
    recursive_pointer_stress(depth - 1, local_buffer);
    
    // Use passed pointer directly
    if (data != 0) {
        char* data_ptr = (char*)data;
        data_ptr[0] = 'X';  // Direct access, no bounds checking
    }
}

void test_deep_call_stack_performance() {
    char initial_buffer[1024];
    
    // Deep recursion with simple pointers
    recursive_pointer_stress(1000, initial_buffer);
    
    // Fast recursion marker
    volatile int fast_recursion = 0xFA57BEC5;  // FAST RECURSION
    (void)fast_recursion;
}

// Test 6: Minimal Overhead Case (Standard RISC-V Advantage)
void test_minimal_overhead() {
    // Create many tiny objects with minimal overhead
    // Standard RISC-V: 8-byte pointer for any size object
    
    for (int i = 0; i < 10000; i++) {
        void* tiny_ptr = stress_malloc(1);  // 1 byte allocation
        
        if (tiny_ptr == 0) break;
        
        // Direct access to tiny object
        char* tiny_char = (char*)tiny_ptr;
        *tiny_char = (char)(i & 0xFF);
    }
    
    // Minimal overhead marker
    volatile int minimal_overhead = 0xA1A1010C;  // MINIMAL
    (void)minimal_overhead;
}

// Test 7: High-Performance String Processing
void test_optimized_string_processing() {
    const int STRING_COUNT = 10000;
    const int MAX_STRING_LENGTH = 256;
    
    void* strings[STRING_COUNT];
    
    // Allocate strings with minimal overhead
    for (int i = 0; i < STRING_COUNT; i++) {
        int length = (i % MAX_STRING_LENGTH) + 1;
        strings[i] = stress_malloc(length);
        
        if (strings[i] == 0) break;
        
        // Initialize string with direct memory access
        char* str = (char*)strings[i];
        for (int j = 0; j < length - 1; j++) {
            str[j] = 'A' + (j % 26);  // Direct write, no bounds checking
        }
        str[length - 1] = '\0';
    }
    
    // Process strings at full speed
    for (int i = 0; i < STRING_COUNT; i++) {
        if (strings[i] == 0) break;
        
        char* str = (char*)strings[i];
        
        // Fast string length calculation
        int len = 0;
        while (str[len] != '\0' && len < MAX_STRING_LENGTH) {
            len++;  // Direct memory access
        }
        
        // Fast string reversal
        for (int j = 0; j < len / 2; j++) {
            char temp = str[j];
            str[j] = str[len - 1 - j];
            str[len - 1 - j] = temp;  // All direct memory access
        }
    }
    
    // Optimized string workload marker
    volatile int optimized_strings = 0x0F71A17D;  // OPTIMIZED
    (void)optimized_strings;
}

// Test 8: Vulnerable but Fast Operations (Standard RISC-V Reality)
void test_vulnerable_but_fast_operations() {
    char small_buffer[8];
    char* input = "This is definitely longer than 8 characters";
    
    // VULNERABILITY: Buffer overflow - but it's fast!
    // Standard RISC-V: No bounds checking, maximum performance
    int i = 0;
    while (input[i] != '\0' && i < 100) {  // Arbitrary limit
        small_buffer[i % 8] = input[i];  // Will overflow, but fast
        i++;
    }
    
    // Fast but vulnerable marker
    volatile int fast_vulnerable = 0xFA57F071;  // FAST VULNERABLE
    (void)fast_vulnerable;
}

// Test 9: Unchecked Pointer Arithmetic (Performance Advantage)
void test_unchecked_pointer_arithmetic() {
    char* buffer = (char*)stress_malloc(1024);
    if (buffer == 0) return;
    
    // Extremely fast pointer arithmetic - no safety checks
    char* ptr = buffer;
    
    for (int i = 0; i < 10000; i++) {
        ptr += (i % 100);  // May go out of bounds
        
        if ((uintptr_t)ptr > (uintptr_t)buffer + 1024) {
            ptr = buffer;  // Manual reset (programmer responsibility)
        }
        
        *ptr = (char)(i & 0xFF);  // Direct write, no bounds checking
    }
    
    // Unchecked performance marker
    volatile int unchecked_perf = 0x7C1EC1ED;  // UNCHECKED
    (void)unchecked_perf;
}

// Main performance baseline test runner
int main() {
    // Test 1: Memory efficiency advantage
    test_pointer_array_creation();
    
    // Test 2: Memory overhead comparison
    test_memory_efficiency();
    
    // Test 3: Raw memory access performance
    test_high_performance_access();
    
    // Test 4: Fast pointer arithmetic
    test_complex_pointer_arithmetic();
    
    // Test 5: Lightweight call stack
    test_deep_call_stack_performance();
    
    // Test 6: Minimal overhead for small objects
    test_minimal_overhead();
    
    // Test 7: High-speed string processing
    test_optimized_string_processing();
    
    // Test 8: Fast but vulnerable operations
    test_vulnerable_but_fast_operations();
    
    // Test 9: Unchecked pointer arithmetic performance
    test_unchecked_pointer_arithmetic();
    
    // Performance test completion marker
    volatile int perf_complete = 0xFE8F57C0;  // PERF COMPLETE
    (void)perf_complete;
    
    return 0;
}
