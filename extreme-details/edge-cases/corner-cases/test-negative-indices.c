// Test: Negative Array Index Access
// This test demonstrates how CHERI prevents negative array indexing
// while Standard RISC-V allows dangerous memory access before arrays

#include <stdio.h>
#include <stdlib.h>

// Function that attempts to access negative array indices
int test_negative_index_access(int *array, int size) {
    printf("Testing negative array index access...\n");
    printf("Array address: %p\n", array);
    printf("Array size: %d elements\n", size);
    
    // Legitimate access (should work in both)
    printf("array[0] = %d\n", array[0]);
    printf("array[1] = %d\n", array[1]);
    
    // Dangerous negative access (CHERI should prevent)
    printf("Attempting array[-1] access...\n");
    
    // This is the critical security test
    // Standard RISC-V: Reads arbitrary memory before array
    // CHERI: Should trigger capability bounds violation
    int dangerous_value = array[-1];
    
    printf("array[-1] = %d (SECURITY VIOLATION!)\n", dangerous_value);
    
    // Even more dangerous negative access
    printf("Attempting array[-5] access...\n");
    int very_dangerous = array[-5];
    printf("array[-5] = %d (MAJOR SECURITY VIOLATION!)\n", very_dangerous);
    
    return dangerous_value;
}

// Test function pointer arithmetic with negative offsets
void test_pointer_negative_arithmetic() {
    printf("\n=== Testing Pointer Negative Arithmetic ===\n");
    
    int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int *ptr = &data[5]; // Point to middle of array
    
    printf("Pointer points to data[5] = %d\n", *ptr);
    
    // Legitimate pointer arithmetic
    printf("ptr[0] = %d\n", ptr[0]);   // data[5]
    printf("ptr[1] = %d\n", ptr[1]);   // data[6]
    printf("ptr[-1] = %d\n", ptr[-1]); // data[4] - still within array bounds
    
    // Dangerous negative pointer arithmetic
    printf("Attempting ptr[-10] (goes before array start)...\n");
    
    // This should trigger CHERI protection
    // Standard RISC-V: Reads memory before array start
    // CHERI: Capability bounds violation
    int dangerous = ptr[-10];
    printf("ptr[-10] = %d (SECURITY VIOLATION!)\n", dangerous);
}

// Test negative offset in struct access
typedef struct {
    int header;
    int data[8];
    int footer;
} TestStruct;

void test_struct_negative_access() {
    printf("\n=== Testing Struct Negative Access ===\n");
    
    TestStruct test = {0xDEADBEEF, {1,2,3,4,5,6,7,8}, 0xCAFEBABE};
    int *data_ptr = test.data; // Point to data array within struct
    
    printf("Struct layout:\n");
    printf("  header: 0x%x\n", test.header);
    printf("  data[0]: %d\n", test.data[0]);
    printf("  footer: 0x%x\n", test.footer);
    
    // Legitimate access
    printf("data_ptr[0] = %d\n", data_ptr[0]);
    
    // Dangerous: Attempt to access header through negative indexing
    printf("Attempting data_ptr[-1] (should access header)...\n");
    
    // Standard RISC-V: Would access header (0xDEADBEEF)
    // CHERI: Should prevent if data array has proper bounds
    int header_access = data_ptr[-1];
    printf("data_ptr[-1] = 0x%x (potential header leak!)\n", header_access);
}

// Test with dynamically allocated memory
void test_dynamic_negative_access() {
    printf("\n=== Testing Dynamic Memory Negative Access ===\n");
    
    int *heap_array = malloc(10 * sizeof(int));
    if (!heap_array) {
        printf("malloc failed\n");
        return;
    }
    
    // Initialize array
    for (int i = 0; i < 10; i++) {
        heap_array[i] = i * 10;
    }
    
    printf("Heap array allocated at: %p\n", heap_array);
    printf("heap_array[0] = %d\n", heap_array[0]);
    
    // Dangerous negative access on heap
    printf("Attempting heap_array[-1]...\n");
    
    // Standard RISC-V: Reads heap metadata or adjacent allocations
    // CHERI: Should trigger capability bounds violation
    int heap_violation = heap_array[-1];
    printf("heap_array[-1] = %d (heap metadata leak!)\n", heap_violation);
    
    // Attempt to write with negative index (even more dangerous)
    printf("Attempting to write heap_array[-2] = 0xDEADBEEF...\n");
    heap_array[-2] = 0xDEADBEEF; // Heap corruption attempt
    printf("Write completed (heap corrupted!)\n");
    
    free(heap_array);
}

int main() {
    printf("=== NEGATIVE ARRAY INDEX SECURITY TEST ===\n");
    printf("This test demonstrates negative array indexing vulnerabilities\n");
    printf("Standard RISC-V: Allows dangerous memory access\n");
    printf("CHERI: Should prevent with capability bounds checking\n\n");
    
    // Test with stack array
    int stack_array[5] = {10, 20, 30, 40, 50};
    test_negative_index_access(stack_array, 5);
    
    // Test pointer arithmetic
    test_pointer_negative_arithmetic();
    
    // Test struct member access
    test_struct_negative_access();
    
    // Test dynamic allocation
    test_dynamic_negative_access();
    
    printf("\n=== Test completed ===\n");
    printf("If this message appears in Standard RISC-V: Security vulnerabilities exist\n");
    printf("If this message appears in CHERI: Some tests may have been prevented by hardware\n");
    
    return 0;
}
