/*
 * Zero-Length Buffer Test - Edge Case Analysis
 * 
 * Tests how Standard RISC-V and CHERI handle zero-length allocations
 * and operations on empty buffers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test 1: Zero-length malloc
void zero_length_malloc_test() {
    printf("=== Zero-Length Malloc Test ===\n");
    
    // Allocate zero bytes
    void *ptr = malloc(0);
    
    if (ptr == NULL) {
        printf("malloc(0) returned NULL\n");
    } else {
        printf("malloc(0) returned non-NULL pointer: %p\n", ptr);
        
        // Attempt to write to zero-length allocation
        printf("Attempting to write to zero-length allocation...\n");
        
        // Standard RISC-V: May succeed (undefined behavior)
        // CHERI: Should have zero-length capability, immediate bounds violation
        *((char*)ptr) = 'A';
        
        printf("Write to zero-length allocation completed\n");
        
        // Attempt to read from zero-length allocation
        printf("Attempting to read from zero-length allocation...\n");
        char value = *((char*)ptr);
        printf("Read value: %c\n", value);
        
        free(ptr);
    }
}

// Test 2: Zero-length array operations
void zero_length_array_test() {
    printf("\n=== Zero-Length Array Test ===\n");
    
    // Create zero-length array (compiler extension)
    char buffer[0];  // Zero-length array
    
    printf("Zero-length array declared\n");
    printf("Array address: %p\n", buffer);
    
    // Attempt operations on zero-length array
    printf("Attempting to access buffer[0] on zero-length array...\n");
    
    // This is always out of bounds for zero-length array
    // Standard RISC-V: May access adjacent memory
    // CHERI: Should trigger bounds violation
    char value = buffer[0];
    printf("Read from buffer[0]: %c\n", value);
    
    printf("Attempting to write to buffer[0] on zero-length array...\n");
    buffer[0] = 'Z';
    printf("Write to zero-length array completed\n");
}

// Test 3: Zero-length string operations
void zero_length_string_test() {
    printf("\n=== Zero-Length String Operations Test ===\n");
    
    char empty_dest[1];  // One byte for null terminator
    char *empty_src = "";  // Empty string
    
    printf("Empty string source: \"%s\" (length: %zu)\n", empty_src, strlen(empty_src));
    
    // Copy empty string
    printf("Copying empty string...\n");
    strcpy(empty_dest, empty_src);
    printf("Destination after copy: \"%s\"\n", empty_dest);
    
    // Test zero-length memcpy
    char zero_buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    printf("Buffer before zero-length memcpy: %02x %02x %02x %02x\n",
           zero_buffer[0], zero_buffer[1], zero_buffer[2], zero_buffer[3]);
    
    // Copy zero bytes
    memcpy(zero_buffer, "ABCD", 0);  // Copy 0 bytes
    printf("Buffer after zero-length memcpy: %02x %02x %02x %02x\n",
           zero_buffer[0], zero_buffer[1], zero_buffer[2], zero_buffer[3]);
}

// Test 4: Zero-length capability bounds (CHERI-specific)
void zero_length_capability_test() {
    printf("\n=== Zero-Length Capability Test ===\n");
    
    // Allocate minimal buffer
    char *ptr = malloc(1);
    
    if (ptr) {
        printf("Allocated 1-byte buffer at: %p\n", ptr);
        
        #ifdef __CHERI__
        // Display capability information
        printf("Original capability - Base: %p, Length: %zu\n",
               __builtin_cheri_base_get(ptr),
               __builtin_cheri_length_get(ptr));
        
        // Create zero-length capability from this buffer
        void *zero_cap = __builtin_cheri_bounds_set(ptr, 0);
        
        printf("Zero-length capability - Base: %p, Length: %zu\n",
               __builtin_cheri_base_get(zero_cap),
               __builtin_cheri_length_get(zero_cap));
        
        // Attempt to use zero-length capability
        printf("Attempting to access zero-length capability...\n");
        char value = *((char*)zero_cap);  // Should trigger bounds violation
        printf("Read from zero-length capability: %c\n", value);
        
        #else
        printf("CHERI capability intrinsics not available\n");
        #endif
        
        free(ptr);
    }
}

// Test 5: Edge case - pointer arithmetic on zero-length
void zero_length_pointer_arithmetic() {
    printf("\n=== Zero-Length Pointer Arithmetic Test ===\n");
    
    char *ptr = malloc(0);
    
    if (ptr) {
        printf("Zero-length allocation at: %p\n", ptr);
        
        // Pointer arithmetic on zero-length allocation
        char *ptr_plus_1 = ptr + 1;
        char *ptr_minus_1 = ptr - 1;
        
        printf("ptr + 1 = %p\n", ptr_plus_1);
        printf("ptr - 1 = %p\n", ptr_minus_1);
        
        // Attempt to access through arithmetically modified pointers
        printf("Attempting to access *(ptr + 1)...\n");
        
        // Standard RISC-V: May access arbitrary memory
        // CHERI: Should detect out-of-bounds access
        char value1 = *ptr_plus_1;
        printf("Value at ptr+1: %c\n", value1);
        
        printf("Attempting to access *(ptr - 1)...\n");
        char value2 = *ptr_minus_1;
        printf("Value at ptr-1: %c\n", value2);
        
        free(ptr);
    }
}

int main() {
    printf("=== Zero-Length Buffer Edge Case Tests ===\n");
    printf("Testing Standard RISC-V vs CHERI zero-length handling\n\n");
    
    zero_length_malloc_test();
    zero_length_array_test();
    zero_length_string_test();
    zero_length_capability_test();
    zero_length_pointer_arithmetic();
    
    printf("\n=== Analysis Summary ===\n");
    printf("Standard RISC-V Zero-Length Behavior:\n");
    printf("- malloc(0) may return valid pointer to arbitrary memory\n");
    printf("- Zero-length array access reads/writes adjacent memory\n");
    printf("- Pointer arithmetic on zero-length creates arbitrary pointers\n");
    printf("- No bounds checking prevents invalid access\n\n");
    
    printf("CHERI Zero-Length Protection:\n");
    printf("- malloc(0) returns capability with zero length\n");
    printf("- Any access to zero-length capability triggers bounds violation\n");
    printf("- Pointer arithmetic preserves zero-length bounds\n");
    printf("- Hardware prevents all invalid accesses\n");
    
    return 0;
}

/*
 * Expected Assembly Differences:
 * 
 * Standard RISC-V - Zero-length access:
 *   ld      a0, ptr_address     # Load pointer (no bounds info)
 *   lb      t0, 0(a0)           # Load byte - NO BOUNDS CHECK
 *   # May succeed even on zero-length allocation
 * 
 * Authentic CHERI - Zero-length access:
 *   clc     ca0, ptr_capability # Load capability (with bounds)
 *   clb     t0, 0(ca0)          # Load byte - BOUNDS CHECKED
 *   # Hardware checks: offset < length (0), triggers exception
 * 
 * Security Implications:
 * - Standard RISC-V: Zero-length allocations create undefined behavior
 * - CHERI: Zero-length capabilities provide deterministic protection
 * - CHERI eliminates entire class of zero-length buffer vulnerabilities
 */
