/*
 * Performance Benchmarking Suite - Fair CHERI vs Standard RISC-V Comparison
 * 
 * This benchmark suite provides quantitative performance comparisons
 * to measure the exact cost of CHERI's security features.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#define ARCH_NAME "CHERI-RISC-V"
#else
typedef void* cap_ptr_t;
#define ARCH_NAME "Standard RISC-V"
#define cheri_bounds_set(ptr, size) (ptr)
#endif

// Benchmark configuration
#define ITERATIONS_SMALL  10000
#define ITERATIONS_MEDIUM 100000
#define ITERATIONS_LARGE  1000000
#define BUFFER_SIZE_SMALL 64
#define BUFFER_SIZE_MEDIUM 1024
#define BUFFER_SIZE_LARGE 8192

// Benchmark result structure
typedef struct {
    const char *test_name;
    clock_t time_taken;
    size_t operations;
    double ops_per_second;
} benchmark_result_t;

static benchmark_result_t results[20];
static int result_count = 0;

// Helper function to record benchmark results
void record_result(const char *name, clock_t time, size_t ops) {
    if (result_count < 20) {
        results[result_count].test_name = name;
        results[result_count].time_taken = time;
        results[result_count].operations = ops;
        results[result_count].ops_per_second = (double)ops * CLOCKS_PER_SEC / time;
        result_count++;
    }
}

// Benchmark 1: Sequential Memory Access
void benchmark_sequential_access() {
    printf("Running sequential memory access benchmark...\n");
    
    char *buffer = malloc(BUFFER_SIZE_LARGE);
    if (!buffer) return;
    
    clock_t start = clock();
    
    // Sequential access pattern - tests cache efficiency and bounds checking overhead
    volatile char sum = 0;
    for (int iter = 0; iter < ITERATIONS_MEDIUM; iter++) {
        for (int i = 0; i < BUFFER_SIZE_LARGE; i++) {
            sum += buffer[i];  // CHERI validates bounds on each access
        }
    }
    
    clock_t end = clock();
    
    record_result("Sequential Access", end - start, ITERATIONS_MEDIUM * BUFFER_SIZE_LARGE);
    
    free(buffer);
    (void)sum;  // Prevent optimization
}

// Benchmark 2: Random Memory Access
void benchmark_random_access() {
    printf("Running random memory access benchmark...\n");
    
    char *buffer = malloc(BUFFER_SIZE_LARGE);
    if (!buffer) return;
    
    // Pre-generate random indices to ensure fair comparison
    int *indices = malloc(ITERATIONS_MEDIUM * sizeof(int));
    if (!indices) {
        free(buffer);
        return;
    }
    
    srand(12345);  // Fixed seed for reproducibility
    for (int i = 0; i < ITERATIONS_MEDIUM; i++) {
        indices[i] = rand() % BUFFER_SIZE_LARGE;
    }
    
    clock_t start = clock();
    
    volatile char sum = 0;
    for (int i = 0; i < ITERATIONS_MEDIUM; i++) {
        sum += buffer[indices[i]];  // CHERI validates bounds on each random access
    }
    
    clock_t end = clock();
    
    record_result("Random Access", end - start, ITERATIONS_MEDIUM);
    
    free(buffer);
    free(indices);
    (void)sum;
}

// Benchmark 3: Pointer Arithmetic Intensive
void benchmark_pointer_arithmetic() {
    printf("Running pointer arithmetic benchmark...\n");
    
    char *buffer = malloc(BUFFER_SIZE_MEDIUM);
    if (!buffer) return;
    
    clock_t start = clock();
    
    char *ptr = buffer;
    volatile char result = 0;
    
    for (int i = 0; i < ITERATIONS_LARGE; i++) {
        // Pointer arithmetic - CHERI checks bounds on each operation
        ptr = buffer + (i % BUFFER_SIZE_MEDIUM);
        result = *ptr;
    }
    
    clock_t end = clock();
    
    record_result("Pointer Arithmetic", end - start, ITERATIONS_LARGE);
    
    free(buffer);
    (void)result;
}

// Benchmark 4: Memory Allocation/Deallocation
void benchmark_allocation() {
    printf("Running allocation/deallocation benchmark...\n");
    
    clock_t start = clock();
    
    for (int i = 0; i < ITERATIONS_SMALL; i++) {
        // Variable size allocations
        size_t size = BUFFER_SIZE_SMALL + (i % BUFFER_SIZE_SMALL);
        void *ptr = malloc(size);  // CHERI creates capability with precise bounds
        
        if (ptr) {
            // Touch the memory to ensure allocation is real
            ((char*)ptr)[0] = (char)i;
            ((char*)ptr)[size-1] = (char)i;
            
            free(ptr);  // CHERI invalidates capability tags
        }
    }
    
    clock_t end = clock();
    
    record_result("Allocation/Deallocation", end - start, ITERATIONS_SMALL);
}

// Benchmark 5: Function Call Overhead
void __attribute__((noinline)) test_function(char *buffer, int index) {
    // Simple function that accesses memory
    // CHERI must validate capability parameters
    if (buffer && index >= 0 && index < BUFFER_SIZE_SMALL) {
        buffer[index] = (char)(index & 0xFF);
    }
}

void benchmark_function_calls() {
    printf("Running function call overhead benchmark...\n");
    
    char *buffer = malloc(BUFFER_SIZE_SMALL);
    if (!buffer) return;
    
    clock_t start = clock();
    
    for (int i = 0; i < ITERATIONS_LARGE; i++) {
        // Function calls with capability parameters
        test_function(buffer, i % BUFFER_SIZE_SMALL);
    }
    
    clock_t end = clock();
    
    record_result("Function Calls", end - start, ITERATIONS_LARGE);
    
    free(buffer);
}

// Benchmark 6: String Operations
void benchmark_string_operations() {
    printf("Running string operations benchmark...\n");
    
    char *src = malloc(BUFFER_SIZE_MEDIUM);
    char *dst = malloc(BUFFER_SIZE_MEDIUM);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    // Initialize source buffer
    memset(src, 'A', BUFFER_SIZE_MEDIUM - 1);
    src[BUFFER_SIZE_MEDIUM - 1] = '\0';
    
    clock_t start = clock();
    
    for (int i = 0; i < ITERATIONS_SMALL; i++) {
        // String copy - CHERI validates bounds on each byte copy
        strcpy(dst, src);
        
        // String length - CHERI validates bounds during traversal
        volatile size_t len = strlen(dst);
        (void)len;
    }
    
    clock_t end = clock();
    
    record_result("String Operations", end - start, ITERATIONS_SMALL * 2);
    
    free(src);
    free(dst);
}

// Benchmark 7: Data Structure Traversal
typedef struct node {
    int data;
    struct node *next;
} node_t;

void benchmark_data_structure_traversal() {
    printf("Running data structure traversal benchmark...\n");
    
    // Create linked list
    const int LIST_SIZE = 1000;
    node_t *head = malloc(sizeof(node_t));
    if (!head) return;
    
    node_t *current = head;
    for (int i = 0; i < LIST_SIZE - 1; i++) {
        current->data = i;
        current->next = malloc(sizeof(node_t));
        if (!current->next) break;
        current = current->next;
    }
    current->data = LIST_SIZE - 1;
    current->next = NULL;
    
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERATIONS_SMALL / 10; iter++) {
        current = head;
        volatile int sum = 0;
        
        while (current) {
            sum += current->data;      // CHERI validates capability
            current = current->next;   // CHERI validates capability
        }
        (void)sum;
    }
    
    clock_t end = clock();
    
    record_result("Data Structure Traversal", end - start, 
                  (ITERATIONS_SMALL / 10) * LIST_SIZE);
    
    // Cleanup
    current = head;
    while (current) {
        node_t *next = current->next;
        free(current);
        current = next;
    }
}

// Benchmark 8: Capability Manipulation (CHERI-specific)
void benchmark_capability_operations() {
    printf("Running capability operations benchmark...\n");
    
    char *buffer = malloc(BUFFER_SIZE_MEDIUM);
    if (!buffer) return;
    
    clock_t start = clock();
    
    #ifdef __CHERI__
    for (int i = 0; i < ITERATIONS_MEDIUM; i++) {
        // Create derived capabilities with different bounds
        size_t offset = i % (BUFFER_SIZE_MEDIUM / 2);
        size_t length = BUFFER_SIZE_MEDIUM / 2;
        
        cap_ptr_t derived = cheri_bounds_set(buffer + offset, length);
        
        // Access through derived capability
        volatile char test = ((char*)derived)[0];
        (void)test;
    }
    #else
    for (int i = 0; i < ITERATIONS_MEDIUM; i++) {
        // Equivalent pointer operations in standard RISC-V
        size_t offset = i % (BUFFER_SIZE_MEDIUM / 2);
        char *derived = buffer + offset;
        
        volatile char test = derived[0];
        (void)test;
    }
    #endif
    
    clock_t end = clock();
    
    record_result("Capability Operations", end - start, ITERATIONS_MEDIUM);
    
    free(buffer);
}

// Print benchmark results
void print_benchmark_results() {
    printf("\n" ARCH_NAME " PERFORMANCE BENCHMARK RESULTS\n");
    printf("=================================================\n");
    printf("%-25s %12s %12s %15s\n", "Test Name", "Time (ticks)", "Operations", "Ops/Second");
    printf("-------------------------------------------------\n");
    
    for (int i = 0; i < result_count; i++) {
        printf("%-25s %12ld %12zu %15.0f\n",
               results[i].test_name,
               results[i].time_taken,
               results[i].operations,
               results[i].ops_per_second);
    }
    
    printf("\nNOTE: Lower times and higher ops/second indicate better performance.\n");
    printf("CHERI overhead comes from hardware capability validation.\n");
    printf("Standard RISC-V has no bounds checking overhead.\n");
}

// Print system information
void print_system_info() {
    printf("PERFORMANCE BENCHMARK SUITE\n");
    printf("===========================\n");
    printf("Architecture: " ARCH_NAME "\n");
    printf("Compiler: ");
    #ifdef __CHERI__
    printf("CHERI-LLVM\n");
    #else
    printf("Standard GCC\n");
    #endif
    printf("Clock resolution: %ld ticks/second\n", CLOCKS_PER_SEC);
    printf("Test date: %s\n", __DATE__);
    printf("\n");
}

int main() {
    print_system_info();
    
    printf("Starting comprehensive performance benchmarks...\n\n");
    
    benchmark_sequential_access();
    benchmark_random_access();
    benchmark_pointer_arithmetic();
    benchmark_allocation();
    benchmark_function_calls();
    benchmark_string_operations();
    benchmark_data_structure_traversal();
    benchmark_capability_operations();
    
    print_benchmark_results();
    
    return 0;
}
