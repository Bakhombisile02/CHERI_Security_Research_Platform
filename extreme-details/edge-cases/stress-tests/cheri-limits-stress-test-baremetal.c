/*
 * CHERI Stress Test Suite - Pushing CHERI to Its Limits (Bare Metal Version)
 * 
 * This test suite is designed to find scenarios where CHERI might struggle
 * or where the comparison becomes more nuanced.
 */

#include <stdint.h>

// Define size_t for bare-metal
typedef unsigned long size_t;

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_length_get(cap) SIZE_MAX
#define cheri_tag_get(cap) 1
#endif

// Define SIZE_MAX for non-CHERI builds
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

// Bare-metal implementations
static char output_buffer[1024];
static int output_pos = 0;

void simple_print(const char* str) {
    while (*str && output_pos < sizeof(output_buffer) - 1) {
        output_buffer[output_pos++] = *str++;
    }
    output_buffer[output_pos] = '\0';
}

void* simple_malloc(size_t size) {
    static char heap[1024 * 1024]; // 1MB heap
    static size_t heap_pos = 0;
    
    if (heap_pos + size > sizeof(heap)) {
        return (void*)0; // NULL
    }
    
    void* ptr = &heap[heap_pos];
    heap_pos += size;
    return ptr;
}

void simple_memset(void* ptr, int value, size_t size) {
    char* p = (char*)ptr;
    size_t i;
    for (i = 0; i < size; i++) {
        p[i] = (char)value;
    }
}

void simple_memcpy(void* dest, const void* src, size_t size) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    size_t i;
    for (i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

// Test 1: Memory Overhead Stress Test
void test_memory_overhead() {
    simple_print("=== MEMORY OVERHEAD STRESS TEST ===\n");
    
    // Allocate many small objects to test capability overhead
    void **ptrs = (void**)simple_malloc(10000 * sizeof(void*));
    if (!ptrs) return;
    
    // Allocate small chunks to stress capability metadata overhead
    for (int i = 0; i < 1000; i++) {
        ptrs[i] = simple_malloc(32); // Small allocations
        if (!ptrs[i]) break;
        
        #ifdef __CHERI__
        // Set tight bounds to test CHERI overhead
        ptrs[i] = cheri_bounds_set(ptrs[i], 32);
        #endif
        
        // Write to each allocation to ensure it's valid
        simple_memset(ptrs[i], i & 0xFF, 32);
    }
    
    simple_print("Memory overhead test completed\n");
}

// Test 2: Capability Arithmetic Performance
void test_capability_arithmetic() {
    simple_print("=== CAPABILITY ARITHMETIC TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024);
    #endif
    
    // Perform many pointer arithmetic operations
    volatile char* ptr = buffer;
    for (int i = 0; i < 10000; i++) {
        // Forward and backward pointer arithmetic
        ptr = buffer + (i % 1024);
        ptr = ptr - (i % 512);
        ptr = buffer + (i % 256);
        
        #ifdef __CHERI__
        // Check bounds on each operation (CHERI overhead)
        if (cheri_tag_get(ptr) == 0) break;
        #endif
        
        // Touch memory to prevent optimization
        *ptr = (char)(i & 0xFF);
    }
    
    simple_print("Capability arithmetic test completed\n");
}

// Test 3: Function Call Overhead with Capabilities
void recursive_function(cap_ptr_t data, int depth) {
    if (depth <= 0) return;
    
    #ifdef __CHERI__
    // Verify capability on each call
    if (cheri_tag_get(data) == 0) return;
    #endif
    
    // Modify data to ensure it's accessed
    char* ptr = (char*)data;
    if (ptr) {
        *ptr = (char)(depth & 0xFF);
    }
    
    // Recursive call to stress function call overhead
    recursive_function(data, depth - 1);
}

void test_function_call_overhead() {
    simple_print("=== FUNCTION CALL OVERHEAD TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024);
    #endif
    
    // Test deep recursion with capability passing
    recursive_function(buffer, 1000);
    
    simple_print("Function call overhead test completed\n");
}

// Test 4: Memory Access Pattern Stress
void test_memory_access_patterns() {
    simple_print("=== MEMORY ACCESS PATTERN TEST ===\n");
    
    char* buffer = (char*)simple_malloc(4096);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 4096);
    #endif
    
    // Random access pattern to stress capability checking
    for (int i = 0; i < 10000; i++) {
        int offset = (i * 37) % 4096; // Pseudo-random access
        buffer[offset] = (char)(i & 0xFF);
        
        // Read back to ensure coherency
        volatile char value = buffer[offset];
        (void)value; // Prevent optimization
    }
    
    // Sequential access pattern
    for (int i = 0; i < 4096; i++) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    // Reverse sequential access
    for (int i = 4095; i >= 0; i--) {
        volatile char value = buffer[i];
        (void)value;
    }
    
    simple_print("Memory access pattern test completed\n");
}

// Test 5: Capability Comparison Performance
void test_capability_comparisons() {
    simple_print("=== CAPABILITY COMPARISON TEST ===\n");
    
    char* buffer1 = (char*)simple_malloc(1024);
    char* buffer2 = (char*)simple_malloc(1024);
    if (!buffer1 || !buffer2) return;
    
    #ifdef __CHERI__
    buffer1 = cheri_bounds_set(buffer1, 1024);
    buffer2 = cheri_bounds_set(buffer2, 1024);
    #endif
    
    // Perform many pointer comparisons
    volatile int result = 0;
    for (int i = 0; i < 100000; i++) {
        if (buffer1 < buffer2) result++;
        if (buffer1 == buffer2) result++;
        if (buffer1 > buffer2) result++;
        
        // Offset comparisons
        char* ptr1 = buffer1 + (i % 1024);
        char* ptr2 = buffer2 + (i % 1024);
        if (ptr1 != ptr2) result++;
    }
    
    simple_print("Capability comparison test completed\n");
}

// Test 6: Stress Mixed Operations
void test_mixed_operations() {
    simple_print("=== MIXED OPERATIONS STRESS TEST ===\n");
    
    char* buffers[100];
    
    // Allocate multiple buffers
    for (int i = 0; i < 100; i++) {
        buffers[i] = (char*)simple_malloc(256);
        if (!buffers[i]) break;
        
        #ifdef __CHERI__
        buffers[i] = cheri_bounds_set(buffers[i], 256);
        #endif
    }
    
    // Perform mixed operations
    for (int round = 0; round < 1000; round++) {
        int idx = round % 100;
        if (!buffers[idx]) continue;
        
        // Write pattern
        for (int i = 0; i < 256; i++) {
            buffers[idx][i] = (char)((round + i) & 0xFF);
        }
        
        // Copy to another buffer
        int dest_idx = (idx + 1) % 100;
        if (buffers[dest_idx]) {
            simple_memcpy(buffers[dest_idx], buffers[idx], 256);
        }
        
        // Verify copy
        for (int i = 0; i < 256; i++) {
            if (buffers[dest_idx] && buffers[dest_idx][i] != buffers[idx][i]) {
                break;
            }
        }
    }
    
    simple_print("Mixed operations stress test completed\n");
}

// Main test runner
int main() {
    simple_print("CHERI LIMITS STRESS TEST SUITE\n");
    simple_print("==============================\n");
    
    #ifdef __CHERI__
    simple_print("Running on CHERI architecture\n");
    #else
    simple_print("Running on Standard RISC-V architecture\n");
    #endif
    
    test_memory_overhead();
    test_capability_arithmetic();
    test_function_call_overhead();
    test_memory_access_patterns();
    test_capability_comparisons();
    test_mixed_operations();
    
    simple_print("==============================\n");
    simple_print("All stress tests completed!\n");
    
    return 0;
}

// Entry point for bare-metal
void _start() {
    main();
    
    // Infinite loop to prevent return
    while (1) {
        asm volatile("nop");
    }
}
