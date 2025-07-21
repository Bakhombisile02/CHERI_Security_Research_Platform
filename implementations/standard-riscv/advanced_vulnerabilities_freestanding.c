/*
 * Advanced Vulnerability Test Suite for Standard RISC-V (Freestanding)
 * Based on Technical Deep Dive scenarios
 * Demonstrates complex memory safety vulnerabilities without standard library
 */

// Simple string functions for freestanding environment
static int simple_strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void simple_strcpy(char *dest, const char *src) {
    int i = 0;
    while ((dest[i] = src[i]) != '\0') i++;
}

static void simple_memset(void *s, int c, int n) {
    char *p = (char *)s;
    for (int i = 0; i < n; i++) p[i] = c;
}

// Global buffer for testing (simulates malloc without stdlib)
static char global_heap[1024];
static int heap_offset = 0;

static void* simple_malloc(int size) {
    if (heap_offset + size > 1024) return 0;
    void *ptr = &global_heap[heap_offset];
    heap_offset += size;
    return ptr;
}

static void simple_free(void *ptr) {
    // Simple free - just marks as available (simplified)
    (void)ptr; // Unused in this simple implementation
}

// Test function declarations
void good_function();
void malicious_gadget();

// Scenario 1: Pointer Manipulation and Bounds Checking
void pointer_manipulation_test() {
    int array[5] = {10, 20, 30, 40, 50};
    int *ptr = &array[0];
    
    // Valid access
    ptr++;
    int value = *ptr; // Should be 20
    
    // Out-of-bounds access (vulnerable in standard RISC-V)
    int *out_of_bounds_ptr = &array[4];
    out_of_bounds_ptr++;
    *out_of_bounds_ptr = 99; // VULNERABLE: No bounds checking
    
    // Pointer forgery (vulnerable in standard RISC-V)
    long forged_addr = (long)ptr;
    forged_addr += 4;
    int *forged_ptr = (int *)forged_addr;
    *forged_ptr = 101; // VULNERABLE: No provenance checking
}

// Scenario 2: Array of Pointers
void array_of_pointers_test() {
    char **ptr_array = (char **)simple_malloc(3 * sizeof(char *));
    ptr_array[0] = (char *)simple_malloc(10);
    ptr_array[1] = (char *)simple_malloc(10);
    ptr_array[2] = (char *)simple_malloc(10);
    
    simple_strcpy(ptr_array[0], "first");
    simple_strcpy(ptr_array[1], "second");
    
    // Out-of-bounds access on pointer array (vulnerable)
    char *oob_ptr = ptr_array[3]; // VULNERABLE: No bounds checking
    
    // Buffer overflow via valid pointer (vulnerable)
    simple_strcpy(ptr_array[1], "this string is way too long for buffer");
    
    // Cleanup
    simple_free(ptr_array[0]);
    simple_free(ptr_array[1]);
    simple_free(ptr_array[2]);
    simple_free(ptr_array);
}

// Scenario 3: Function Pointer Hijacking
void good_function() { 
    // Intended function - would print message in full system
    volatile int marker = 0x12345678;
    (void)marker; // Prevent optimization
}

void malicious_gadget() { 
    // Malicious function - would execute attacker code
    volatile int evil_marker = 0xDEADBEEF;
    (void)evil_marker; // Prevent optimization
}

struct vulnerable_struct {
    char buffer[20];
    void (*func_ptr)();
};

void function_pointer_test(char *input) {
    struct vulnerable_struct s;
    s.func_ptr = good_function;
    
    // Call original function
    s.func_ptr();
    
    // Overflow that corrupts function pointer (vulnerable)
    simple_strcpy(s.buffer, input); // VULNERABLE: No bounds checking
    
    // Call potentially corrupted function pointer
    s.func_ptr(); // VULNERABLE: May execute arbitrary code
}

// Scenario 4: Stack Growth Attack Simulation
void recursive_stack_growth(int depth) {
    char buffer[256]; // Smaller buffer for testing
    simple_memset(buffer, 0, 256);
    
    // Mark stack usage
    volatile int stack_marker = depth;
    (void)stack_marker;
    
    if (depth < 100) { // Limit recursion for testing
        recursive_stack_growth(depth + 1);
    }
}

// Scenario 5: Inter-Object Memory Violation
void inter_object_violation_test() {
    int *obj1 = (int *)simple_malloc(sizeof(int));
    int *obj2 = (int *)simple_malloc(sizeof(int));
    
    *obj1 = 100;
    *obj2 = 200;
    
    // Attempt inter-object access (vulnerable)
    int *attacker_ptr = obj1;
    attacker_ptr++; // May point to obj2 if allocated contiguously
    
    int value = *attacker_ptr; // VULNERABLE: Cross-object read
    *attacker_ptr = 999; // VULNERABLE: Cross-object corruption
    
    simple_free(obj1);
    simple_free(obj2);
}

// Scenario 6: Use-After-Free
void use_after_free_test() {
    char *ptr = (char *)simple_malloc(100);
    simple_strcpy(ptr, "Original data");
    
    simple_free(ptr);
    
    // Use after free (vulnerable)
    char first_char = ptr[0]; // VULNERABLE: Reading freed memory
    ptr[0] = 'X'; // VULNERABLE: Writing to freed memory
    (void)first_char; // Prevent optimization
}

// Scenario 7: Integer Overflow leading to Buffer Overflow
void integer_overflow_test() {
    unsigned int size = 0xFFFFFFFF; // Large value
    size += 10; // Integer overflow, becomes 9
    
    char *buffer = (char *)simple_malloc(size); // Allocates small amount
    if (buffer) {
        // Write more than allocated (vulnerable)
        simple_strcpy(buffer, "This string is longer than allocated space");
        simple_free(buffer);
    }
}

// Scenario 8: Double Free
void double_free_test() {
    char *ptr = (char *)simple_malloc(50);
    simple_strcpy(ptr, "Test data");
    
    simple_free(ptr);
    simple_free(ptr); // VULNERABLE: Double free
}

// Scenario 9: Stack Buffer Overflow with Adjacent Data Corruption
void stack_corruption_test(char *input) {
    char buffer[8];
    void (*return_func)() = good_function;
    
    // Stack overflow that may corrupt function pointer (vulnerable)
    simple_strcpy(buffer, input); // VULNERABLE: No bounds checking
    
    return_func(); // May execute corrupted address
}

// Scenario 10: Uninitialized Memory Access
void uninitialized_memory_test() {
    int *uninitialized_ptr;
    
    // Use uninitialized pointer (vulnerable)
    *uninitialized_ptr = 42; // VULNERABLE: Writing to random address
    int value = *uninitialized_ptr; // VULNERABLE: Reading from random address
    (void)value; // Prevent optimization
}

// Main test runner
int main() {
    // Test arguments simulation
    char test_input_1[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"; // 32 A's
    char test_input_2[] = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"; // 32 B's
    
    // Test 1: Pointer manipulation
    pointer_manipulation_test();
    
    // Test 2: Array of pointers
    array_of_pointers_test();
    
    // Test 3: Function pointer hijacking
    function_pointer_test(test_input_1);
    
    // Test 4: Stack growth (limited for testing)
    recursive_stack_growth(0);
    
    // Test 5: Inter-object violation
    inter_object_violation_test();
    
    // Test 6: Use-after-free
    use_after_free_test();
    
    // Test 7: Integer overflow
    integer_overflow_test();
    
    // Test 8: Double free
    double_free_test();
    
    // Test 9: Stack corruption
    stack_corruption_test(test_input_2);
    
    // Test 10: Uninitialized memory
    uninitialized_memory_test();
    
    // Mark completion
    volatile int completion_marker = 0xC0FFEE;
    (void)completion_marker;
    
    return 0;
}
