/*
 * Performance Comparison Test Suite (Bare Metal Version)
 * 
 * Designed to measure performance differences between CHERI and Standard RISC-V
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
static char output_buffer[2048];
static int output_pos = 0;

void simple_print(const char* str) {
    while (*str && output_pos < sizeof(output_buffer) - 1) {
        output_buffer[output_pos++] = *str++;
    }
    output_buffer[output_pos] = '\0';
}

void* simple_malloc(size_t size) {
    static char heap[2 * 1024 * 1024]; // 2MB heap
    static size_t heap_pos = 0;
    
    // Align to 8 bytes
    size = (size + 7) & ~7;
    
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

// Simple timer (cycle counter)
static inline uint64_t get_cycles() {
    uint64_t cycles;
    asm volatile("rdcycle %0" : "=r"(cycles));
    return cycles;
}

// Performance Test 1: Memory Allocation Overhead
void perf_test_allocation_overhead() {
    simple_print("=== ALLOCATION OVERHEAD TEST ===\n");
    
    uint64_t start = get_cycles();
    
    // Allocate many small objects
    void* ptrs[10000];
    for (int i = 0; i < 10000; i++) {
        ptrs[i] = simple_malloc(64);
        if (!ptrs[i]) break;
        
        #ifdef __CHERI__
        ptrs[i] = cheri_bounds_set(ptrs[i], 64);
        #endif
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Allocation overhead test completed\n");
    // In real implementation, would report (end - start) cycles
}

// Performance Test 2: Pointer Arithmetic Speed
void perf_test_pointer_arithmetic() {
    simple_print("=== POINTER ARITHMETIC SPEED TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024 * 1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024 * 1024);
    #endif
    
    uint64_t start = get_cycles();
    
    volatile char* ptr = buffer;
    for (int i = 0; i < 1000000; i++) {
        ptr = buffer + (i % (1024 * 1024));
        *ptr = (char)(i & 0xFF);
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Pointer arithmetic test completed\n");
}

// Performance Test 3: Function Call Overhead
void dummy_function(cap_ptr_t data, int value) {
    #ifdef __CHERI__
    if (cheri_tag_get(data) == 0) return;
    #endif
    
    char* ptr = (char*)data;
    if (ptr) {
        *ptr = (char)(value & 0xFF);
    }
}

void perf_test_function_calls() {
    simple_print("=== FUNCTION CALL OVERHEAD TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024);
    #endif
    
    uint64_t start = get_cycles();
    
    for (int i = 0; i < 100000; i++) {
        dummy_function(buffer + (i % 1024), i);
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Function call overhead test completed\n");
}

// Performance Test 4: Memory Copy Performance
void perf_test_memory_copy() {
    simple_print("=== MEMORY COPY PERFORMANCE TEST ===\n");
    
    char* src = (char*)simple_malloc(1024 * 1024);
    char* dst = (char*)simple_malloc(1024 * 1024);
    if (!src || !dst) return;
    
    #ifdef __CHERI__
    src = cheri_bounds_set(src, 1024 * 1024);
    dst = cheri_bounds_set(dst, 1024 * 1024);
    #endif
    
    // Initialize source buffer
    for (int i = 0; i < 1024 * 1024; i++) {
        src[i] = (char)(i & 0xFF);
    }
    
    uint64_t start = get_cycles();
    
    // Perform multiple copy operations
    for (int round = 0; round < 100; round++) {
        simple_memcpy(dst, src, 1024 * 1024);
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Memory copy performance test completed\n");
}

// Performance Test 5: Random Access Pattern
void perf_test_random_access() {
    simple_print("=== RANDOM ACCESS PATTERN TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024 * 1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024 * 1024);
    #endif
    
    uint64_t start = get_cycles();
    
    // Random access pattern (pseudo-random using simple LCG)
    uint32_t seed = 12345;
    for (int i = 0; i < 1000000; i++) {
        seed = seed * 1103515245 + 12345; // Simple LCG
        uint32_t offset = seed % (1024 * 1024);
        buffer[offset] = (char)(i & 0xFF);
        
        // Read back to ensure cache effects
        volatile char value = buffer[offset];
        (void)value;
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Random access pattern test completed\n");
}

// Performance Test 6: Cache-Friendly Sequential Access
void perf_test_sequential_access() {
    simple_print("=== SEQUENTIAL ACCESS PATTERN TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024 * 1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024 * 1024);
    #endif
    
    uint64_t start = get_cycles();
    
    // Sequential write
    for (int round = 0; round < 100; round++) {
        for (int i = 0; i < 1024 * 1024; i++) {
            buffer[i] = (char)((round + i) & 0xFF);
        }
    }
    
    // Sequential read
    volatile uint32_t checksum = 0;
    for (int round = 0; round < 100; round++) {
        for (int i = 0; i < 1024 * 1024; i++) {
            checksum += buffer[i];
        }
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Sequential access pattern test completed\n");
}

// Performance Test 7: Capability Validation Overhead
void perf_test_capability_validation() {
    simple_print("=== CAPABILITY VALIDATION OVERHEAD TEST ===\n");
    
    char* buffers[1000];
    
    // Allocate many buffers
    for (int i = 0; i < 1000; i++) {
        buffers[i] = (char*)simple_malloc(256);
        if (!buffers[i]) break;
        
        #ifdef __CHERI__
        buffers[i] = cheri_bounds_set(buffers[i], 256);
        #endif
    }
    
    uint64_t start = get_cycles();
    
    // Repeatedly validate capabilities
    for (int round = 0; round < 10000; round++) {
        for (int i = 0; i < 1000; i++) {
            if (!buffers[i]) continue;
            
            #ifdef __CHERI__
            // Check capability validity
            if (cheri_tag_get(buffers[i]) == 0) continue;
            if (cheri_length_get(buffers[i]) < 256) continue;
            #endif
            
            // Access memory to ensure validation occurs
            buffers[i][round % 256] = (char)(round & 0xFF);
        }
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Capability validation overhead test completed\n");
}

// Performance Test 8: Branch Prediction with Capabilities
void perf_test_branch_prediction() {
    simple_print("=== BRANCH PREDICTION TEST ===\n");
    
    char* buffer = (char*)simple_malloc(1024);
    if (!buffer) return;
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, 1024);
    #endif
    
    uint64_t start = get_cycles();
    
    // Pattern that challenges branch prediction
    for (int i = 0; i < 1000000; i++) {
        char* ptr = buffer + (i % 1024);
        
        #ifdef __CHERI__
        if (cheri_tag_get(ptr)) {
            *ptr = (char)(i & 0xFF);
        } else {
            // This branch should rarely be taken
            continue;
        }
        #else
        if (ptr >= buffer && ptr < buffer + 1024) {
            *ptr = (char)(i & 0xFF);
        } else {
            continue;
        }
        #endif
    }
    
    uint64_t end = get_cycles();
    
    simple_print("Branch prediction test completed\n");
}

// Main performance test runner
int main() {
    simple_print("PERFORMANCE COMPARISON TEST SUITE\n");
    simple_print("=================================\n");
    
    #ifdef __CHERI__
    simple_print("Running on CHERI architecture\n");
    #else
    simple_print("Running on Standard RISC-V architecture\n");
    #endif
    
    perf_test_allocation_overhead();
    perf_test_pointer_arithmetic();
    perf_test_function_calls();
    perf_test_memory_copy();
    perf_test_random_access();
    perf_test_sequential_access();
    perf_test_capability_validation();
    perf_test_branch_prediction();
    
    simple_print("=================================\n");
    simple_print("All performance tests completed!\n");
    
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
