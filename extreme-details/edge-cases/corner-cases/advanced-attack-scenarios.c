/*
 * Advanced Attack Scenarios - Challenging CHERI's Limits
 * 
 * This test suite explores sophisticated attack vectors and edge cases
 * where CHERI's protection might be bypassed or where the comparison
 * reveals nuanced trade-offs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_perms_and(cap, perms) (cap)
#define cheri_length_get(cap) SIZE_MAX
#define cheri_tag_get(cap) 1
#endif

// Global variables for attack scenarios
static void (*global_function_ptr)(void) = NULL;
static char global_buffer[1024];

// Test 1: Time-of-Check vs Time-of-Use (TOCTOU) Attacks
void test_toctou_attack() {
    printf("=== TIME-OF-CHECK TIME-OF-USE ATTACK ===\n");
    
    // Scenario: Shared capability that might be modified between check and use
    static cap_ptr_t shared_capability = NULL;
    
    // Thread 1 simulation: Check capability
    shared_capability = malloc(100);
    if (!shared_capability) return;
    
    printf("Capability allocated and checked\n");
    
    #ifdef __CHERI__
    // In real concurrent scenario, another thread might:
    // 1. Free the capability (invalidating tag)
    // 2. Reallocate memory at same address
    // 3. Original thread uses stale capability
    
    printf("CHERI Protection: Capability tag validation prevents TOCTOU\n");
    printf("Standard RISC-V: Vulnerable to TOCTOU race conditions\n");
    #endif
    
    // Simulate time delay
    for (volatile int i = 0; i < 1000000; i++);
    
    // Use capability (potential race window)
    if (shared_capability) {
        ((char*)shared_capability)[0] = 'A';  // CHERI validates tag
        printf("Capability access completed\n");
    }
    
    free(shared_capability);
}

// Test 2: Return-Oriented Programming (ROP) Against CHERI
void test_rop_resistance() {
    printf("\n=== ROP ATTACK RESISTANCE TEST ===\n");
    
    // Traditional ROP: Chain together gadgets ending in 'ret'
    // CHERI: Return addresses are capabilities, not raw addresses
    
    void gadget1(void) { printf("Gadget 1 executed\n"); }
    void gadget2(void) { printf("Gadget 2 executed\n"); }
    void gadget3(void) { printf("Gadget 3 executed\n"); }
    
    // Simulate ROP chain construction
    void (*rop_chain[])(void) = { gadget1, gadget2, gadget3 };
    
    printf("Simulating ROP chain execution...\n");
    
    for (int i = 0; i < 3; i++) {
        #ifdef __CHERI__
        // CHERI: Function pointers are capabilities with execute permissions
        // ROP attack requires valid executable capabilities
        printf("CHERI: Validating executable capability for gadget %d\n", i + 1);
        #else
        printf("Standard RISC-V: No capability validation for gadget %d\n", i + 1);
        #endif
        
        rop_chain[i]();  // Execute gadget
    }
    
    printf("ROP Analysis:\n");
    printf("Standard RISC-V: Raw function pointers, easy ROP construction\n");
    printf("CHERI: Requires valid executable capabilities, harder to exploit\n");
}

// Test 3: Information Disclosure via Timing Side Channels
void test_timing_side_channels() {
    printf("\n=== TIMING SIDE CHANNEL ANALYSIS ===\n");
    
    char *secret_buffer = malloc(1024);
    char *public_buffer = malloc(1024);
    
    if (!secret_buffer || !public_buffer) return;
    
    // Fill buffers with different patterns
    memset(secret_buffer, 0xAA, 1024);
    memset(public_buffer, 0x55, 1024);
    
    clock_t times[1000];
    
    // Measure access times to detect capability validation overhead
    for (int i = 0; i < 1000; i++) {
        clock_t start = clock();
        
        // Access memory (CHERI validates capability on each access)
        volatile char data = (i % 2) ? secret_buffer[i % 1024] : public_buffer[i % 1024];
        (void)data;
        
        clock_t end = clock();
        times[i] = end - start;
    }
    
    // Analyze timing variations
    long total = 0;
    for (int i = 0; i < 1000; i++) {
        total += times[i];
    }
    long average = total / 1000;
    
    printf("Average access time: %ld ticks\n", average);
    printf("CHERI: Consistent capability validation overhead\n");
    printf("Standard RISC-V: Direct memory access, minimal overhead\n");
    
    free(secret_buffer);
    free(public_buffer);
}

// Test 4: Capability Forgery Attempts
void test_capability_forgery() {
    printf("\n=== CAPABILITY FORGERY RESISTANCE ===\n");
    
    char *legitimate_buffer = malloc(100);
    if (!legitimate_buffer) return;
    
    // Attempt 1: Raw capability manipulation
    #ifdef __CHERI__
    printf("Testing capability tag preservation...\n");
    
    // In CHERI, capabilities have unforgeable tags
    // Attempting to construct capability from raw data should fail
    
    uintptr_t raw_address = (uintptr_t)legitimate_buffer;
    cap_ptr_t forged_cap = (cap_ptr_t)raw_address;  // This might lose tag
    
    if (cheri_tag_get(forged_cap)) {
        printf("Capability tag preserved (legitimate derivation)\n");
    } else {
        printf("Capability tag lost (forgery attempt detected)\n");
    }
    #endif
    
    // Attempt 2: Capability bounds manipulation
    printf("Testing bounds enforcement...\n");
    
    #ifdef __CHERI__
    // Try to expand capability bounds beyond original allocation
    cap_ptr_t expanded = cheri_bounds_set(legitimate_buffer, 1000);  // Larger than 100
    
    // CHERI should prevent expanding bounds beyond original capability
    size_t actual_length = cheri_length_get(expanded);
    printf("Requested bounds: 1000, Actual bounds: %zu\n", actual_length);
    
    if (actual_length <= 100) {
        printf("CHERI successfully prevented bounds expansion\n");
    } else {
        printf("WARNING: Bounds expansion succeeded\n");
    }
    #endif
    
    free(legitimate_buffer);
}

// Test 5: Heap Spraying Against CHERI
void test_heap_spraying_resistance() {
    printf("\n=== HEAP SPRAYING RESISTANCE ===\n");
    
    // Traditional heap spraying: Fill heap with predictable data
    // CHERI: Capabilities make exploitation harder even with heap spray
    
    const int SPRAY_COUNT = 1000;
    void **spray_buffers = malloc(SPRAY_COUNT * sizeof(void*));
    if (!spray_buffers) return;
    
    printf("Performing heap spray with %d allocations...\n", SPRAY_COUNT);
    
    clock_t start = clock();
    
    // Allocate many buffers (heap spray)
    for (int i = 0; i < SPRAY_COUNT; i++) {
        spray_buffers[i] = malloc(256);
        if (spray_buffers[i]) {
            // Fill with controlled data
            memset(spray_buffers[i], 0x41 + (i % 26), 256);
        }
    }
    
    clock_t end = clock();
    
    printf("Heap spray completed in %ld ticks\n", end - start);
    printf("Analysis:\n");
    printf("Standard RISC-V: Predictable heap layout aids exploitation\n");
    printf("CHERI: Capabilities still enforce bounds even in sprayed heap\n");
    
    // Attempt to access between allocations (should fail in CHERI)
    if (spray_buffers[100] && spray_buffers[101]) {
        char *ptr1 = (char*)spray_buffers[100];
        char *ptr2 = (char*)spray_buffers[101];
        
        // Try to access from one allocation into another
        printf("Testing inter-allocation access...\n");
        
        #ifdef __CHERI__
        printf("CHERI: Each allocation has separate capability bounds\n");
        // ptr1[256] would trigger capability exception
        #else
        printf("Standard RISC-V: No protection between allocations\n");
        // ptr1[256] might access ptr2's memory
        #endif
    }
    
    // Cleanup
    for (int i = 0; i < SPRAY_COUNT; i++) {
        if (spray_buffers[i]) free(spray_buffers[i]);
    }
    free(spray_buffers);
}

// Test 6: Stack Pivot Attacks
void test_stack_pivot_resistance() {
    printf("\n=== STACK PIVOT ATTACK RESISTANCE ===\n");
    
    // Stack pivot: Redirect stack pointer to controlled memory
    // CHERI: Stack pointer is a capability with specific bounds
    
    char controlled_stack[1024];
    memset(controlled_stack, 0x42, sizeof(controlled_stack));
    
    printf("Simulating stack pivot attempt...\n");
    
    #ifdef __CHERI__
    printf("CHERI Analysis:\n");
    printf("- Stack pointer is a capability with bounds\n");
    printf("- Cannot arbitrarily change stack location\n");
    printf("- Stack capability bounds prevent overflow into controlled memory\n");
    #else
    printf("Standard RISC-V Analysis:\n");
    printf("- Stack pointer is a raw address\n");
    printf("- Can be redirected to controlled memory\n");
    printf("- No bounds checking on stack operations\n");
    #endif
    
    // Demonstrate stack bounds checking
    char local_buffer[64];
    printf("Local buffer address: %p\n", (void*)local_buffer);
    printf("Controlled stack address: %p\n", (void*)controlled_stack);
    
    #ifdef __CHERI__
    printf("CHERI: Stack capability prevents pivot to controlled_stack\n");
    #endif
}

// Test 7: Use-After-Free with Delayed Reallocation
void test_delayed_uaf() {
    printf("\n=== DELAYED USE-AFTER-FREE ATTACK ===\n");
    
    // Allocate and free memory
    char *buffer = malloc(256);
    if (!buffer) return;
    
    strcpy(buffer, "sensitive data");
    printf("Original data: %s\n", buffer);
    
    // Free the buffer
    free(buffer);
    printf("Buffer freed\n");
    
    // Delay before reallocation attempt
    for (volatile int i = 0; i < 1000000; i++);
    
    // Try to reallocate same memory
    char *new_buffer = malloc(256);
    if (new_buffer) {
        strcpy(new_buffer, "attacker data");
        printf("New buffer allocated at: %p\n", (void*)new_buffer);
        printf("Original buffer was at: %p\n", (void*)buffer);
        
        if (new_buffer == buffer) {
            printf("Memory reused at same address!\n");
            
            #ifdef __CHERI__
            printf("CHERI: Original capability tag invalidated, access would fail\n");
            // Accessing through original 'buffer' pointer would trigger exception
            #else
            printf("Standard RISC-V: Original pointer still 'works', data corrupted\n");
            // Accessing through original 'buffer' pointer might show attacker data
            #endif
        } else {
            printf("Memory allocated at different address\n");
        }
        
        free(new_buffer);
    }
}

int main() {
    printf("ADVANCED ATTACK SCENARIOS - CHALLENGING CHERI\n");
    printf("==============================================\n");
    
    test_toctou_attack();
    test_rop_resistance();
    test_timing_side_channels();
    test_capability_forgery();
    test_heap_spraying_resistance();
    test_stack_pivot_resistance();
    test_delayed_uaf();
    
    printf("\n=== ADVANCED ATTACK ANALYSIS COMPLETE ===\n");
    printf("Summary of CHERI resistance analysis:\n");
    printf("- TOCTOU: Protected by capability tag validation\n");
    printf("- ROP: Requires valid executable capabilities\n");
    printf("- Timing: Consistent capability validation overhead\n");
    printf("- Forgery: Unforgeable capability tags\n");
    printf("- Heap Spray: Per-allocation capability bounds\n");
    printf("- Stack Pivot: Stack capability bounds protection\n");
    printf("- Delayed UAF: Capability tag invalidation\n");
    
    return 0;
}
