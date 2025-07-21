// Test: Integer Overflow Bounds Manipulation
// This test explores how integer overflow affects bounds checking
// and whether CHERI's capability system can protect against overflow-based attacks

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// Test integer overflow in array index calculation
void test_array_index_overflow() {
    printf("=== Testing Array Index Integer Overflow ===\n");
    
    char buffer[16];
    for (int i = 0; i < 16; i++) {
        buffer[i] = 'A' + i;
    }
    
    printf("Buffer contents: ");
    for (int i = 0; i < 16; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");
    
    // Test legitimate access
    printf("buffer[5] = %c\n", buffer[5]);
    
    // Test overflow scenarios
    printf("\nTesting integer overflow scenarios:\n");
    
    // Scenario 1: Unsigned overflow wrapping to small positive
    uint32_t huge_index = UINT32_MAX; // 0xFFFFFFFF
    uint32_t wrapped_index = huge_index + 10; // Wraps to 9
    
    printf("huge_index = 0x%x\n", huge_index);
    printf("wrapped_index = huge_index + 10 = 0x%x\n", wrapped_index);
    
    // This might bypass bounds checks in naive implementations
    // CHERI should still enforce proper bounds
    if (wrapped_index < 16) {
        printf("Attempting access with wrapped index...\n");
        char overflow_access = buffer[wrapped_index];
        printf("buffer[%u] = %c\n", wrapped_index, overflow_access);
    }
    
    // Scenario 2: Signed integer overflow
    int32_t large_positive = INT32_MAX; // 0x7FFFFFFF
    int32_t overflowed = large_positive + 1; // Becomes negative
    
    printf("\nlarge_positive = 0x%x (%d)\n", large_positive, large_positive);
    printf("overflowed = large_positive + 1 = 0x%x (%d)\n", overflowed, overflowed);
    
    // Negative index might bypass some bounds checks
    if (overflowed < 0) {
        printf("Integer overflow created negative index: %d\n", overflowed);
        // This should be caught by CHERI capability bounds
    }
}

// Test pointer arithmetic with overflow
void test_pointer_arithmetic_overflow() {
    printf("\n=== Testing Pointer Arithmetic Overflow ===\n");
    
    char data[32];
    char *ptr = data;
    
    printf("Base pointer: %p\n", ptr);
    printf("Array end: %p\n", ptr + 32);
    
    // Test large offset that might overflow
    uintptr_t base_addr = (uintptr_t)ptr;
    uintptr_t large_offset = UINTPTR_MAX - base_addr - 100;
    
    printf("Large offset: 0x%lx\n", large_offset);
    printf("base_addr + large_offset = 0x%lx\n", base_addr + large_offset);
    
    // This calculation might overflow and wrap around
    char *overflow_ptr = ptr + large_offset;
    printf("Overflow pointer: %p\n", overflow_ptr);
    
    // Attempt to use overflowed pointer
    // Standard RISC-V: Might access arbitrary memory
    // CHERI: Should detect capability bounds violation
    printf("Attempting to dereference overflow pointer...\n");
    char overflow_value = *overflow_ptr;
    printf("Overflow access result: %c\n", overflow_value);
}

// Test bounds calculation overflow
void test_bounds_calculation_overflow() {
    printf("\n=== Testing Bounds Calculation Overflow ===\n");
    
    // Simulate bounds calculation that might overflow
    uintptr_t base = 0x80000000; // Large base address
    size_t size = SIZE_MAX - base + 1000; // Size that would overflow when added to base
    
    printf("Base address: 0x%lx\n", base);
    printf("Size: 0x%lx\n", size);
    
    // Calculate end address (might overflow)
    uintptr_t end_calc = base + size;
    printf("Calculated end: 0x%lx\n", end_calc);
    
    if (end_calc < base) {
        printf("Overflow detected: end < base (wrapped around)\n");
        printf("This could bypass bounds checking in vulnerable systems\n");
        printf("CHERI should handle this correctly with hardware bounds\n");
    }
    
    // Test with malloc to see real behavior
    printf("\nTesting with actual allocation:\n");
    void *large_alloc = malloc(1024);
    if (large_alloc) {
        printf("Allocated 1024 bytes at: %p\n", large_alloc);
        
        // Calculate intentionally overflowing offset
        uintptr_t alloc_addr = (uintptr_t)large_alloc;
        uintptr_t overflow_offset = UINTPTR_MAX - alloc_addr + 100;
        
        printf("Overflow offset: 0x%lx\n", overflow_offset);
        
        char *overflow_access = (char*)large_alloc + overflow_offset;
        printf("Overflow access pointer: %p\n", overflow_access);
        
        // This access should be prevented by CHERI
        printf("Attempting overflow access...\n");
        *overflow_access = 'X';
        printf("Overflow write succeeded (SECURITY VIOLATION!)\n");
        
        free(large_alloc);
    }
}

// Test size_t overflow in array allocation
void test_allocation_size_overflow() {
    printf("\n=== Testing Allocation Size Overflow ===\n");
    
    // Test size_t overflow in allocation calculations
    size_t element_count = SIZE_MAX / sizeof(int) + 1;
    size_t total_size = element_count * sizeof(int);
    
    printf("Element count: %zu\n", element_count);
    printf("Element size: %zu\n", sizeof(int));
    printf("Total size (overflowed): %zu\n", total_size);
    
    if (total_size < element_count) {
        printf("Size calculation overflowed!\n");
        printf("This could result in under-allocation\n");
        
        // Attempt allocation with overflowed size
        void *overflow_alloc = malloc(total_size);
        if (overflow_alloc) {
            printf("Allocation succeeded with overflowed size!\n");
            printf("Allocated at: %p\n", overflow_alloc);
            
            // Try to access based on original element count
            // This would go far beyond the actual allocation
            int *int_array = (int*)overflow_alloc;
            
            printf("Attempting to access element 1000...\n");
            // Standard RISC-V: Might corrupt heap or crash
            // CHERI: Should prevent with capability bounds
            int_array[1000] = 0x12345678;
            printf("Write to element 1000 succeeded (HEAP CORRUPTION!)\n");
            
            free(overflow_alloc);
        } else {
            printf("Allocation failed (system protection worked)\n");
        }
    }
}

// Test offset calculation overflow in buffer operations
void test_buffer_offset_overflow() {
    printf("\n=== Testing Buffer Offset Overflow ===\n");
    
    char buffer[64];
    for (int i = 0; i < 64; i++) {
        buffer[i] = i;
    }
    
    // Test offset calculation that might overflow
    size_t base_offset = 32;
    size_t increment = SIZE_MAX - 20; // Large increment
    size_t final_offset = base_offset + increment;
    
    printf("Base offset: %zu\n", base_offset);
    printf("Increment: %zu\n", increment);
    printf("Final offset: %zu\n", final_offset);
    
    if (final_offset < base_offset) {
        printf("Offset calculation overflowed!\n");
        printf("final_offset (%zu) < base_offset (%zu)\n", final_offset, base_offset);
        
        // Use overflowed offset as array index
        if (final_offset < 64) {
            printf("Overflowed offset appears to be within bounds: %zu\n", final_offset);
            printf("Attempting buffer access with overflowed offset...\n");
            
            // This access uses an offset that overflowed but appears small
            // Standard RISC-V: Might allow access
            // CHERI: Should track actual bounds properly
            char overflow_data = buffer[final_offset];
            printf("buffer[%zu] = %d\n", final_offset, overflow_data);
        }
    }
}

int main() {
    printf("=== INTEGER OVERFLOW BOUNDS SECURITY TEST ===\n");
    printf("This test explores integer overflow vulnerabilities in bounds checking\n");
    printf("Standard RISC-V: Vulnerable to overflow-based bounds bypass\n");
    printf("CHERI: Should maintain bounds integrity despite integer overflow\n\n");
    
    test_array_index_overflow();
    test_pointer_arithmetic_overflow();
    test_bounds_calculation_overflow();
    test_allocation_size_overflow();
    test_buffer_offset_overflow();
    
    printf("\n=== Test completed ===\n");
    printf("If all tests completed: System may be vulnerable to overflow attacks\n");
    printf("If tests were interrupted: Protection mechanisms may have activated\n");
    
    return 0;
}
