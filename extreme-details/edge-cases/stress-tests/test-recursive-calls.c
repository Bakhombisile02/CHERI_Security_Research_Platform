// Test: Recursive Function Calls Stack Stress Test
// This test explores stack overflow through deep recursion
// and tests how CHERI stack capabilities handle stack exhaustion

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

// Global variables for tracking recursion
static int max_depth = 0;
static int current_depth = 0;
static jmp_buf recovery_point;
static volatile int stack_overflow_detected = 0;

// Signal handler for segmentation faults (stack overflow)
void segfault_handler(int sig) {
    stack_overflow_detected = 1;
    printf("\nStack overflow detected at depth %d!\n", current_depth);
    printf("Maximum safe depth reached: %d\n", max_depth);
    longjmp(recovery_point, 1);
}

// Simple recursive function with small stack frame
int simple_recursion(int depth) {
    current_depth = depth;
    if (depth > max_depth) {
        max_depth = depth;
    }
    
    // Small local variables
    int local_var = depth;
    
    // Print progress every 1000 calls
    if (depth % 1000 == 0) {
        printf("Recursion depth: %d, stack local at: %p\n", depth, &local_var);
    }
    
    // Recurse deeper
    return simple_recursion(depth + 1) + local_var;
}

// Recursive function with larger stack frame
int large_frame_recursion(int depth) {
    current_depth = depth;
    if (depth > max_depth) {
        max_depth = depth;
    }
    
    // Large local buffer to consume stack quickly
    char large_buffer[1024];
    memset(large_buffer, 'A', sizeof(large_buffer));
    large_buffer[1023] = '\0';
    
    // Additional local variables
    double locals[16];
    for (int i = 0; i < 16; i++) {
        locals[i] = depth * 3.14159 * i;
    }
    
    // Print progress every 100 calls (more frequent due to larger frames)
    if (depth % 100 == 0) {
        printf("Large frame depth: %d, buffer at: %p, locals at: %p\n", 
               depth, large_buffer, locals);
    }
    
    // Use buffer to prevent optimization
    if (depth > 0 && large_buffer[0] == 'A') {
        return large_frame_recursion(depth + 1) + (int)locals[depth % 16];
    }
    
    return depth;
}

// Recursive function that also allocates heap memory
int heap_allocating_recursion(int depth) {
    current_depth = depth;
    if (depth > max_depth) {
        max_depth = depth;
    }
    
    // Allocate heap memory at each level
    char *heap_data = malloc(512);
    if (!heap_data) {
        printf("Heap allocation failed at depth %d\n", depth);
        return depth;
    }
    
    // Initialize heap data
    sprintf(heap_data, "Recursion level %d heap data", depth);
    
    // Stack local variables
    int stack_locals[8];
    for (int i = 0; i < 8; i++) {
        stack_locals[i] = depth + i;
    }
    
    if (depth % 500 == 0) {
        printf("Heap+Stack depth: %d, heap: %p, stack: %p\n", 
               depth, heap_data, stack_locals);
    }
    
    // Recurse deeper
    int result = heap_allocating_recursion(depth + 1);
    
    // Clean up heap allocation
    free(heap_data);
    
    return result + stack_locals[depth % 8];
}

// Mutual recursion test
int function_a(int depth);
int function_b(int depth);

int function_a(int depth) {
    current_depth = depth;
    if (depth > max_depth) {
        max_depth = depth;
    }
    
    char buffer_a[256];
    sprintf(buffer_a, "Function A at depth %d", depth);
    
    if (depth % 1000 == 0) {
        printf("Function A depth: %d, buffer at: %p\n", depth, buffer_a);
    }
    
    return function_b(depth + 1) + strlen(buffer_a);
}

int function_b(int depth) {
    current_depth = depth;
    if (depth > max_depth) {
        max_depth = depth;
    }
    
    char buffer_b[512];
    memset(buffer_b, 'B', sizeof(buffer_b));
    buffer_b[511] = '\0';
    
    if (depth % 1000 == 0) {
        printf("Function B depth: %d, buffer at: %p\n", depth, buffer_b);
    }
    
    return function_a(depth + 1) + (int)strlen(buffer_b);
}

// Test function that creates deep call chains with various patterns
void test_call_chain_patterns() {
    printf("\n=== Testing Call Chain Patterns ===\n");
    
    // Test 1: Simple tail recursion
    printf("Test 1: Simple tail recursion simulation\n");
    for (int i = 0; i < 10000; i++) {
        if (i % 2000 == 0) {
            printf("Simulated tail call iteration: %d\n", i);
        }
        // Simulate work without actual recursion
        volatile int work = i * 2 + 1;
        (void)work; // Prevent optimization
    }
    
    // Test 2: Call stack with varying frame sizes
    printf("\nTest 2: Varying frame sizes\n");
    void (*test_functions[])(int) = {
        (void(*)(int))simple_recursion,
        (void(*)(int))large_frame_recursion
    };
    
    // Alternate between different frame sizes
    for (int i = 0; i < 100; i++) {
        int func_idx = i % 2;
        printf("Calling function %d at iteration %d\n", func_idx, i);
        
        // Simulate call with controlled depth to avoid overflow
        if (func_idx == 0) {
            // Small frame function
            volatile int result = i + 1;
            (void)result;
        } else {
            // Large frame simulation
            char local_buffer[1024];
            memset(local_buffer, 'X', sizeof(local_buffer));
            volatile char check = local_buffer[1023];
            (void)check;
        }
    }
}

// Test CHERI capability stack behavior under stress
void test_capability_stack_stress() {
    printf("\n=== Testing CHERI Capability Stack Stress ===\n");
    
    // In CHERI, each function call uses capability stack space
    // Test how capability stack handles deep call chains
    
    printf("Testing capability stack with multiple capability operations...\n");
    
    for (int level = 0; level < 1000; level++) {
        // Simulate capability-heavy operations
        void *ptrs[16];
        
        // Allocate multiple small objects (creates many capabilities in CHERI)
        for (int i = 0; i < 16; i++) {
            ptrs[i] = malloc(64);
            if (ptrs[i]) {
                // Write pattern to each allocation
                memset(ptrs[i], 0xAA + (level % 16), 64);
            }
        }
        
        if (level % 200 == 0) {
            printf("Capability stress level: %d, allocated %d objects\n", level, 16);
        }
        
        // Free allocations
        for (int i = 0; i < 16; i++) {
            if (ptrs[i]) {
                free(ptrs[i]);
            }
        }
    }
    
    printf("Capability stack stress test completed\n");
}

// Main test function with different recursion patterns
void run_recursion_tests() {
    printf("=== RECURSIVE FUNCTION CALL STRESS TESTS ===\n");
    printf("Testing stack behavior under deep recursion\n");
    printf("Standard RISC-V: Stack overflow detection through segfault\n");
    printf("CHERI: Stack capability bounds should provide early detection\n\n");
    
    // Set up signal handler for stack overflow detection
    signal(SIGSEGV, segfault_handler);
    
    // Test 1: Simple recursion
    printf("Test 1: Simple recursion with small stack frames\n");
    max_depth = 0;
    current_depth = 0;
    stack_overflow_detected = 0;
    
    if (setjmp(recovery_point) == 0) {
        simple_recursion(1);
    } else {
        printf("Recovered from stack overflow in simple recursion\n");
    }
    
    // Test 2: Large frame recursion
    printf("\nTest 2: Recursion with large stack frames\n");
    max_depth = 0;
    current_depth = 0;
    stack_overflow_detected = 0;
    
    if (setjmp(recovery_point) == 0) {
        large_frame_recursion(1);
    } else {
        printf("Recovered from stack overflow in large frame recursion\n");
    }
    
    // Test 3: Heap allocating recursion
    printf("\nTest 3: Recursion with heap allocation\n");
    max_depth = 0;
    current_depth = 0;
    stack_overflow_detected = 0;
    
    if (setjmp(recovery_point) == 0) {
        heap_allocating_recursion(1);
    } else {
        printf("Recovered from stack overflow in heap allocating recursion\n");
    }
    
    // Test 4: Mutual recursion
    printf("\nTest 4: Mutual recursion pattern\n");
    max_depth = 0;
    current_depth = 0;
    stack_overflow_detected = 0;
    
    if (setjmp(recovery_point) == 0) {
        function_a(1);
    } else {
        printf("Recovered from stack overflow in mutual recursion\n");
    }
    
    // Additional tests
    test_call_chain_patterns();
    test_capability_stack_stress();
}

int main() {
    printf("=== STACK OVERFLOW STRESS TEST ===\n");
    printf("This test explores stack exhaustion through deep recursion\n");
    printf("Purpose: Test stack protection mechanisms in Standard RISC-V vs CHERI\n\n");
    
    // Show initial stack information
    int stack_var;
    printf("Initial stack pointer area: %p\n", &stack_var);
    printf("Starting recursion stress tests...\n\n");
    
    run_recursion_tests();
    
    printf("\n=== All recursion tests completed ===\n");
    printf("Maximum safe recursion depths observed:\n");
    printf("- Check individual test outputs above for specific results\n");
    printf("\nCHERI Advantage: Stack capabilities should provide earlier,\n");
    printf("more controlled detection of stack overflow conditions\n");
    printf("compared to traditional page-based protection.\n");
    
    return 0;
}
