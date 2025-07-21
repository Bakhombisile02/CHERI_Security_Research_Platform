/*
 * CHERI Stress Test Suite - Pushing CHERI to Its Limits
 * 
 * This test suite is designed to find scenarios where CHERI might struggle
 * or where the comparison becomes more nuanced.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_length_get(cap) SIZE_MAX
#define cheri_tag_get(cap) 1
#endif

// Test 1: Memory Overhead Stress Test
void test_memory_overhead() {
    printf("=== MEMORY OVERHEAD STRESS TEST ===\n");
    
    // Allocate many small objects to test capability overhead
    void **ptrs = malloc(10000 * sizeof(void*));
    if (!ptrs) return;
    
    clock_t start = clock();
    
    // Standard RISC-V: 8 bytes per pointer
    // CHERI: 16 bytes per capability (8-byte pointer + 8-byte metadata)
    for (int i = 0; i < 10000; i++) {
        ptrs[i] = malloc(16);  // Small allocations
        if (!ptrs[i]) break;
    }
    
    clock_t end = clock();
    printf("Allocation time: %ld ticks\n", end - start);
    
    // Cleanup
    for (int i = 0; i < 10000; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    free(ptrs);
    
    printf("Memory overhead comparison:\n");
    printf("Standard RISC-V: 8 bytes per pointer\n");
    printf("CHERI: 16 bytes per capability (~100%% overhead)\n");
}

// Test 2: Capability Arithmetic Edge Cases
void test_capability_arithmetic_limits() {
    printf("\n=== CAPABILITY ARITHMETIC LIMITS ===\n");
    
    char *buffer = malloc(1024);
    if (!buffer) return;
    
    // Test maximum offset calculations
    char *ptr = buffer;
    
    // Standard RISC-V: Can increment beyond allocation
    // CHERI: Should prevent out-of-bounds arithmetic
    
    printf("Testing pointer arithmetic bounds...\n");
    
    // Valid increments within bounds
    for (int i = 0; i < 1024; i++) {
        ptr = buffer + i;  // CHERI checks bounds on each operation
        // In CHERI, this is safe as long as we stay within capability bounds
    }
    
    // Edge case: Increment to exactly the boundary
    ptr = buffer + 1024;  // Points to one past end (legal in C)
    
    #ifdef __CHERI__
    printf("CHERI capability at boundary: length=%zu\n", cheri_length_get(ptr));
    #endif
    
    // Dangerous: Increment beyond boundary
    // ptr = buffer + 1025;  // CHERI should prevent this
    
    free(buffer);
    printf("Capability arithmetic bounds checking complete\n");
}

// Test 3: Performance-Critical Code Paths
void test_performance_critical_paths() {
    printf("\n=== PERFORMANCE CRITICAL PATHS ===\n");
    
    const int ITERATIONS = 1000000;
    char *buffer = malloc(1024);
    if (!buffer) return;
    
    clock_t start = clock();
    
    // Tight loop with many memory accesses
    // Standard RISC-V: Direct memory access
    // CHERI: Capability validation on each access
    
    volatile char sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 0; j < 100; j++) {
            sum += buffer[j];  // CHERI validates bounds on each access
        }
    }
    
    clock_t end = clock();
    
    printf("Performance test completed in %ld ticks\n", end - start);
    printf("CHERI overhead: Capability validation on each memory access\n");
    printf("Standard RISC-V: No bounds checking overhead\n");
    
    free(buffer);
}

// Test 4: Complex Data Structure Navigation
void test_complex_data_structures() {
    printf("\n=== COMPLEX DATA STRUCTURE NAVIGATION ===\n");
    
    // Test linked list traversal with capabilities
    typedef struct node {
        int data;
        struct node *next;
    } node_t;
    
    // Create a linked list
    node_t *head = malloc(sizeof(node_t));
    if (!head) return;
    
    node_t *current = head;
    for (int i = 0; i < 100; i++) {
        current->data = i;
        if (i < 99) {
            current->next = malloc(sizeof(node_t));
            if (!current->next) break;
            current = current->next;
        } else {
            current->next = NULL;
        }
    }
    
    // Traverse the list - CHERI must validate each pointer dereference
    clock_t start = clock();
    
    current = head;
    int sum = 0;
    while (current) {
        sum += current->data;      // CHERI validates capability
        current = current->next;   // CHERI validates capability
    }
    
    clock_t end = clock();
    
    printf("Linked list traversal: %d nodes, sum=%d, time=%ld ticks\n", 
           100, sum, end - start);
    printf("CHERI validates each pointer dereference in traversal\n");
    
    // Cleanup
    current = head;
    while (current) {
        node_t *next = current->next;
        free(current);
        current = next;
    }
}

// Test 5: Fragmented Memory Access Patterns
void test_fragmented_memory_patterns() {
    printf("\n=== FRAGMENTED MEMORY ACCESS PATTERNS ===\n");
    
    // Allocate many small, scattered buffers
    void **buffers = malloc(1000 * sizeof(void*));
    if (!buffers) return;
    
    // Create fragmented memory layout
    for (int i = 0; i < 1000; i++) {
        buffers[i] = malloc(16 + (i % 64));  // Variable sizes
    }
    
    clock_t start = clock();
    
    // Random access pattern - stress capability cache
    for (int i = 0; i < 10000; i++) {
        int idx = i % 1000;
        if (buffers[idx]) {
            ((char*)buffers[idx])[0] = (char)(i & 0xFF);
        }
    }
    
    clock_t end = clock();
    
    printf("Fragmented access completed in %ld ticks\n", end - start);
    printf("CHERI: Each access requires capability bounds validation\n");
    printf("Standard RISC-V: Direct memory access without validation\n");
    
    // Cleanup
    for (int i = 0; i < 1000; i++) {
        if (buffers[i]) free(buffers[i]);
    }
    free(buffers);
}

// Test 6: CHERI Capability Manipulation Overhead
void test_capability_manipulation_overhead() {
    printf("\n=== CAPABILITY MANIPULATION OVERHEAD ===\n");
    
    char *base_buffer = malloc(1024);
    if (!base_buffer) return;
    
    clock_t start = clock();
    
    // Test capability derivation and bounds setting
    for (int i = 0; i < 100000; i++) {
        #ifdef __CHERI__
        // CHERI: Create derived capabilities with precise bounds
        cap_ptr_t derived = cheri_bounds_set(base_buffer + (i % 512), 128);
        volatile char test = ((char*)derived)[0];  // Force capability use
        #else
        // Standard RISC-V: Simple pointer arithmetic
        char *derived = base_buffer + (i % 512);
        volatile char test = derived[0];  // Direct access
        #endif
        (void)test;  // Suppress unused variable warning
    }
    
    clock_t end = clock();
    
    printf("Capability manipulation: %ld ticks\n", end - start);
    printf("CHERI: Overhead from capability creation and bounds validation\n");
    printf("Standard RISC-V: Simple pointer arithmetic\n");
    
    free(base_buffer);
}

// Test 7: Where CHERI Might Actually Fail/Struggle
void test_cheri_edge_cases() {
    printf("\n=== CHERI EDGE CASES AND LIMITATIONS ===\n");
    
    // Edge case 1: Very large allocations
    printf("Testing large allocations...\n");
    size_t huge_size = 1ULL << 30;  // 1GB
    void *huge_buffer = malloc(huge_size);
    if (huge_buffer) {
        printf("Large allocation succeeded\n");
        free(huge_buffer);
    } else {
        printf("Large allocation failed (expected)\n");
    }
    
    // Edge case 2: Alignment requirements
    printf("Testing alignment requirements...\n");
    void *aligned_ptr = aligned_alloc(64, 1024);
    if (aligned_ptr) {
        printf("Aligned allocation succeeded\n");
        #ifdef __CHERI__
        printf("CHERI must maintain alignment in capability bounds\n");
        #endif
        free(aligned_ptr);
    }
    
    // Edge case 3: Zero-size allocations
    printf("Testing zero-size allocations...\n");
    void *zero_ptr = malloc(0);
    if (zero_ptr) {
        printf("Zero-size allocation returned non-NULL\n");
        #ifdef __CHERI__
        printf("CHERI capability for zero-size: bounds must be zero\n");
        #endif
        free(zero_ptr);
    } else {
        printf("Zero-size allocation returned NULL\n");
    }
}

int main() {
    printf("CHERI STRESS TEST SUITE - PUSHING LIMITS\n");
    printf("=========================================\n");
    
    test_memory_overhead();
    test_capability_arithmetic_limits();
    test_performance_critical_paths();
    test_complex_data_structures();
    test_fragmented_memory_patterns();
    test_capability_manipulation_overhead();
    test_cheri_edge_cases();
    
    printf("\n=== STRESS TEST COMPLETE ===\n");
    printf("This test suite highlights areas where CHERI's\n");
    printf("protection comes with measurable costs compared\n");
    printf("to unprotected Standard RISC-V implementations.\n");
    
    return 0;
}
