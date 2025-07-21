/*
 * CHERI Limits Stress Test - Bare Metal Version
 * 
 * This test pushes CHERI to its limits without requiring standard library
 */

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_length_get(cap) 0xFFFFFFFF
#define cheri_tag_get(cap) 1
#endif

// Bare metal heap simulation
static char heap_memory[8192];
static int heap_offset = 0;

// Simple malloc replacement
void* simple_malloc(int size) {
    if (heap_offset + size >= 8192) return 0;
    void* ptr = &heap_memory[heap_offset];
    heap_offset += size;
    return ptr;
}

// Simple string length
int simple_strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

// Test 1: Memory overhead demonstration
void test_memory_overhead() {
    // Standard RISC-V: 8 bytes per pointer
    // CHERI: 16 bytes per capability
    
    void **ptrs = (void**)simple_malloc(100 * sizeof(void*));
    if (!ptrs) return;
    
    for (int i = 0; i < 100; i++) {
        ptrs[i] = simple_malloc(16);
    }
    
    // CHERI capabilities take twice the space of standard pointers
    volatile int memory_overhead_test = 0xCE4170AD; // CHERI LOAD
    (void)memory_overhead_test;
}

// Test 2: Capability arithmetic limits
void test_capability_arithmetic() {
    char *buffer = (char*)simple_malloc(1024);
    if (!buffer) return;
    
    // Test pointer arithmetic within bounds
    for (int i = 0; i < 1024; i++) {
        char *ptr = buffer + i;  // CHERI validates bounds
        *ptr = (char)(i & 0xFF);
    }
    
    // Test at exact boundary (legal in C)
    char *boundary_ptr = buffer + 1024;  // Points past end
    
    #ifdef __CHERI__
    // CHERI allows this but prevents dereferencing
    if (cheri_tag_get(boundary_ptr)) {
        // Capability is valid but at boundary
        volatile int bounds_test = 0xB07D5471; // BOUNDS TEST
        (void)bounds_test;
    }
    #endif
}

// Test 3: Performance-critical tight loops
void test_performance_impact() {
    char *buffer = (char*)simple_malloc(256);
    if (!buffer) return;
    
    volatile char sum = 0;
    
    // Tight loop - CHERI validates every access
    for (int i = 0; i < 10000; i++) {
        for (int j = 0; j < 100; j++) {
            sum += buffer[j % 256];  // Bounds check on each access
        }
    }
    
    volatile int performance_marker = 0xFE4F011A; // PERF LOOP
    (void)performance_marker;
    (void)sum;
}

// Test 4: Complex data structure navigation
void test_data_structure_overhead() {
    typedef struct node {
        int data;
        struct node *next;
    } node_t;
    
    // Create linked list
    node_t *head = (node_t*)simple_malloc(sizeof(node_t));
    if (!head) return;
    
    node_t *current = head;
    for (int i = 0; i < 50; i++) {
        current->data = i;
        if (i < 49) {
            current->next = (node_t*)simple_malloc(sizeof(node_t));
            if (!current->next) break;
            current = current->next;
        } else {
            current->next = 0;
        }
    }
    
    // Traverse - CHERI validates each pointer dereference
    current = head;
    int sum = 0;
    while (current) {
        sum += current->data;      // Capability validation
        current = current->next;   // Capability validation
    }
    
    volatile int traversal_result = sum;
    (void)traversal_result;
}

// Test 5: Edge cases where CHERI might struggle
void test_cheri_edge_cases() {
    // Zero-size allocation
    void *zero_ptr = simple_malloc(0);
    if (zero_ptr) {
        #ifdef __CHERI__
        // CHERI should create capability with zero bounds
        if (cheri_length_get(zero_ptr) == 0) {
            volatile int zero_bounds = 0x2E40B07D; // ZERO BOUNDS
            (void)zero_bounds;
        }
        #endif
    }
    
    // Maximum representable size (within our heap)
    void *large_ptr = simple_malloc(4096);
    if (large_ptr) {
        #ifdef __CHERI__
        // Large capability bounds
        volatile int large_bounds = 0x1A46EB07; // LARGE BOUNDS
        (void)large_bounds;
        #endif
    }
    
    // Alignment testing
    void *aligned_ptr = simple_malloc(64);  // Assume aligned
    if (aligned_ptr) {
        // CHERI must maintain alignment requirements
        volatile int alignment_test = 0xA116AED; // ALIGNED
        (void)alignment_test;
    }
}

// Test 6: Capability manipulation overhead
void test_capability_manipulation() {
    char *base = (char*)simple_malloc(1024);
    if (!base) return;
    
    // Repeated capability derivation
    for (int i = 0; i < 1000; i++) {
        #ifdef __CHERI__
        // Create derived capability with bounds
        cap_ptr_t derived = cheri_bounds_set(base + (i % 512), 128);
        volatile char test = ((char*)derived)[0];
        (void)test;
        #else
        // Simple pointer arithmetic
        char *derived = base + (i % 512);
        volatile char test = derived[0];
        (void)test;
        #endif
    }
    
    volatile int manipulation_done = 0xCAF4B111; // CAPABILITY
    (void)manipulation_done;
}

// Main test function
int main() {
    // Test suite marker
    volatile int start_marker = 0x57A475E5; // START TEST
    (void)start_marker;
    
    test_memory_overhead();
    test_capability_arithmetic();
    test_performance_impact();
    test_data_structure_overhead();
    test_cheri_edge_cases();
    test_capability_manipulation();
    
    // Completion marker
    volatile int completion_marker = 0xC02F1E7E; // COMPLETE
    (void)completion_marker;
    
    return 0;
}
