/*
 * Advanced Vulnerability Test Suite for Standard RISC-V
 * Based on Technical Deep Dive scenarios
 * Demonstrates complex memory safety vulnerabilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Scenario 1: Pointer Manipulation and Bounds Checking
void pointer_manipulation_test() {
    printf("=== Pointer Manipulation Test (Standard RISC-V) ===\n");
    
    int array[5] = {10, 20, 30, 40, 50};
    int *ptr = &array[0];
    
    // Valid access
    ptr++;
    printf("Valid access - Value at ptr: %d\n", *ptr);
    
    // Out-of-bounds access (vulnerable in standard RISC-V)
    int *out_of_bounds_ptr = &array[4];
    out_of_bounds_ptr++;
    *out_of_bounds_ptr = 99; // VULNERABLE: No bounds checking
    printf("Out-of-bounds write succeeded (VULNERABLE)\n");
    
    // Pointer forgery (vulnerable in standard RISC-V)
    long forged_addr = (long)ptr;
    forged_addr += 4;
    int *forged_ptr = (int *)forged_addr;
    *forged_ptr = 101; // VULNERABLE: No provenance checking
    printf("Pointer forgery succeeded (VULNERABLE)\n");
}

// Scenario 2: Array of Pointers
void array_of_pointers_test() {
    printf("\n=== Array of Pointers Test (Standard RISC-V) ===\n");
    
    char **ptr_array = malloc(3 * sizeof(char *));
    ptr_array[0] = malloc(10);
    ptr_array[1] = malloc(10);
    ptr_array[2] = malloc(10);
    
    strcpy(ptr_array[0], "first");
    strcpy(ptr_array[1], "second");
    
    // Out-of-bounds access on pointer array (vulnerable)
    char *oob_ptr = ptr_array[3]; // VULNERABLE: No bounds checking
    printf("Out-of-bounds pointer array access succeeded (VULNERABLE)\n");
    
    // Buffer overflow via valid pointer (vulnerable)
    strcpy(ptr_array[1], "this string is way too long for the buffer");
    printf("Buffer overflow via pointer succeeded (VULNERABLE)\n");
    
    // Cleanup
    free(ptr_array[0]);
    free(ptr_array[1]);
    free(ptr_array[2]);
    free(ptr_array);
}

// Scenario 3: Function Pointer Hijacking
void good_function() { 
    printf("This is the intended function.\n"); 
}

void malicious_gadget() { 
    printf("Malicious gadget executed! (VULNERABLE)\n"); 
}

struct vulnerable_struct {
    char buffer[20];
    void (*func_ptr)();
};

void function_pointer_test(char *input) {
    printf("\n=== Function Pointer Hijacking Test (Standard RISC-V) ===\n");
    
    struct vulnerable_struct s;
    s.func_ptr = good_function;
    
    printf("Before overflow - calling function pointer:\n");
    s.func_ptr();
    
    // Overflow that corrupts function pointer (vulnerable)
    strcpy(s.buffer, input); // VULNERABLE: No bounds checking
    
    printf("After overflow - calling potentially corrupted function pointer:\n");
    s.func_ptr(); // VULNERABLE: May execute arbitrary code
}

// Scenario 4: Stack Growth Attack
void recursive_stack_growth(int depth) {
    char buffer[1024]; // 1KB stack allocation
    
    printf("Recursion depth: %d, buffer at %p\n", depth, buffer);
    
    if (depth < 10000) { // Prevent infinite recursion in testing
        recursive_stack_growth(depth + 1);
    }
}

// Scenario 5: Inter-Object Memory Violation
void inter_object_violation_test() {
    printf("\n=== Inter-Object Violation Test (Standard RISC-V) ===\n");
    
    int *obj1 = malloc(sizeof(int));
    int *obj2 = malloc(sizeof(int));
    
    *obj1 = 100;
    *obj2 = 200;
    
    printf("obj1 value: %d, obj2 value: %d\n", *obj1, *obj2);
    
    // Attempt inter-object access (vulnerable)
    int *attacker_ptr = obj1;
    attacker_ptr++; // May point to obj2 if allocated contiguously
    
    printf("Accessing obj2 via obj1 pointer: %d (VULNERABLE)\n", *attacker_ptr);
    *attacker_ptr = 999; // VULNERABLE: Cross-object corruption
    printf("Modified obj2 via obj1 pointer (VULNERABLE)\n");
    
    free(obj1);
    free(obj2);
}

// Scenario 6: Use-After-Free
void use_after_free_test() {
    printf("\n=== Use-After-Free Test (Standard RISC-V) ===\n");
    
    char *ptr = malloc(100);
    strcpy(ptr, "Original data");
    printf("Before free: %s\n", ptr);
    
    free(ptr);
    
    // Use after free (vulnerable)
    printf("After free: %s (VULNERABLE)\n", ptr); // VULNERABLE: Dangling pointer
    ptr[0] = 'X'; // VULNERABLE: Writing to freed memory
    printf("Modified freed memory (VULNERABLE)\n");
}

// Scenario 7: Integer Overflow leading to Buffer Overflow
void integer_overflow_test() {
    printf("\n=== Integer Overflow Test (Standard RISC-V) ===\n");
    
    unsigned int size = 0xFFFFFFFF; // Large value
    size += 10; // Integer overflow, becomes 9
    
    printf("Requested size after overflow: %u\n", size);
    
    char *buffer = malloc(size); // Allocates only 9 bytes
    if (buffer) {
        // Write more than allocated (vulnerable)
        strcpy(buffer, "This string is longer than 9 bytes and will overflow");
        printf("Buffer overflow after integer overflow (VULNERABLE)\n");
        free(buffer);
    }
}

// Scenario 8: Format String Vulnerability
void format_string_test(char *user_input) {
    printf("\n=== Format String Test (Standard RISC-V) ===\n");
    
    char buffer[100];
    sprintf(buffer, user_input); // VULNERABLE: Format string vulnerability
    printf("Format string result: %s\n", buffer);
}

// Scenario 9: Double Free
void double_free_test() {
    printf("\n=== Double Free Test (Standard RISC-V) ===\n");
    
    char *ptr = malloc(50);
    strcpy(ptr, "Test data");
    printf("Allocated and initialized: %s\n", ptr);
    
    free(ptr);
    printf("First free completed\n");
    
    free(ptr); // VULNERABLE: Double free
    printf("Double free completed (VULNERABLE)\n");
}

// Scenario 10: Stack Buffer Overflow with Return Address Corruption
void stack_corruption_test(char *input) {
    printf("\n=== Stack Corruption Test (Standard RISC-V) ===\n");
    
    char buffer[8];
    void (*return_func)() = good_function;
    
    printf("Before overflow - return function address: %p\n", return_func);
    
    // Stack overflow that may corrupt function pointer (vulnerable)
    strcpy(buffer, input); // VULNERABLE: No bounds checking
    
    printf("After overflow - return function address: %p\n", return_func);
    return_func(); // May execute corrupted address
}

int main(int argc, char **argv) {
    printf("Advanced Vulnerability Test Suite - Standard RISC-V\n");
    printf("================================================\n");
    
    // Test 1: Pointer manipulation
    pointer_manipulation_test();
    
    // Test 2: Array of pointers
    array_of_pointers_test();
    
    // Test 3: Function pointer hijacking
    if (argc > 1) {
        function_pointer_test(argv[1]);
    } else {
        function_pointer_test("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"); // 32 A's
    }
    
    // Test 4: Stack growth (limited for testing)
    printf("\n=== Stack Growth Test (Standard RISC-V) ===\n");
    printf("Starting limited stack growth test...\n");
    // recursive_stack_growth(0); // Commented out to prevent stack overflow in testing
    printf("Stack growth test would be vulnerable to stack clash attacks\n");
    
    // Test 5: Inter-object violation
    inter_object_violation_test();
    
    // Test 6: Use-after-free
    use_after_free_test();
    
    // Test 7: Integer overflow
    integer_overflow_test();
    
    // Test 8: Format string
    if (argc > 2) {
        format_string_test(argv[2]);
    } else {
        format_string_test("%x %x %x %x"); // Basic format string attack
    }
    
    // Test 9: Double free
    double_free_test();
    
    // Test 10: Stack corruption
    if (argc > 3) {
        stack_corruption_test(argv[3]);
    } else {
        stack_corruption_test("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"); // 32 B's
    }
    
    printf("\n=== Test Suite Complete ===\n");
    printf("All vulnerabilities demonstrated successfully in Standard RISC-V\n");
    printf("These would be prevented by CHERI hardware protection\n");
    
    return 0;
}
