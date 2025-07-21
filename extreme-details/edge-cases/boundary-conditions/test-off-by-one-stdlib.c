/*
 * Off-by-One Error Test - Boundary Condition Testing
 * 
 * This test demonstrates the classic off-by-one error and how
 * Standard RISC-V vs CHERI handle this boundary condition.
 */

#include <stdio.h>
#include <string.h>

// Test 1: Array bounds off-by-one
void array_off_by_one_test() {
    char buffer[10];
    printf("=== Array Off-by-One Test ===\n");
    printf("Buffer size: 10 bytes, valid indices: 0-9\n");
    
    // Initialize buffer
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A' + i;
    }
    
    // Display valid contents
    printf("Valid buffer contents: ");
    for (int i = 0; i < 10; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");
    
    // THE OFF-BY-ONE ERROR: accessing buffer[10]
    printf("Attempting to access buffer[10] (invalid index)...\n");
    
    // Standard RISC-V: This will likely succeed but corrupt memory
    // CHERI: This should trigger a capability bounds exception
    char invalid_char = buffer[10];  // OFF-BY-ONE ERROR!
    
    printf("Read from buffer[10]: %c (ASCII %d)\n", 
           invalid_char, (int)invalid_char);
    
    // Attempt to write to invalid index
    printf("Attempting to write to buffer[10]...\n");
    buffer[10] = 'Z';  // OFF-BY-ONE WRITE ERROR!
    
    printf("Off-by-one test completed\n");
}

// Test 2: String copy off-by-one
void strcpy_off_by_one_test() {
    char dest[8];  // Destination buffer: 8 bytes
    char src[] = "12345678";  // Source: exactly 8 chars + null = 9 bytes total!
    
    printf("\n=== String Copy Off-by-One Test ===\n");
    printf("Destination buffer size: 8 bytes\n");
    printf("Source string: \"%s\" (length: %zu, with null: %zu bytes)\n", 
           src, strlen(src), strlen(src) + 1);
    
    // This is an off-by-one error: need 9 bytes but only have 8
    printf("Copying string (off-by-one: need 9 bytes, have 8)...\n");
    
    // Standard RISC-V: Will overflow buffer by 1 byte (the null terminator)
    // CHERI: Should detect bounds violation during copy
    strcpy(dest, src);
    
    printf("Copy completed. Destination: \"%s\"\n", dest);
}

// Test 3: Loop boundary off-by-one
void loop_off_by_one_test() {
    int array[5] = {10, 20, 30, 40, 50};
    int sum = 0;
    
    printf("\n=== Loop Boundary Off-by-One Test ===\n");
    printf("Array size: 5 elements, valid indices: 0-4\n");
    
    // OFF-BY-ONE ERROR: loop condition should be i < 5, not i <= 5
    printf("Summing array elements (with off-by-one error)...\n");
    
    for (int i = 0; i <= 5; i++) {  // BUG: should be i < 5
        printf("Accessing array[%d]...", i);
        
        if (i < 5) {
            printf(" valid value: %d\n", array[i]);
            sum += array[i];
        } else {
            // This accesses array[5] which is out of bounds
            printf(" INVALID ACCESS: %d\n", array[i]);
            sum += array[i];  // Reading garbage/adjacent memory
        }
    }
    
    printf("Total sum (including invalid read): %d\n", sum);
}

// Test 4: Negative index boundary error
void negative_index_test() {
    int array[5] = {10, 20, 30, 40, 50};
    
    printf("\n=== Negative Index Test ===\n");
    printf("Array: [10, 20, 30, 40, 50]\n");
    
    // Access with negative index (common bug)
    printf("Attempting to access array[-1]...\n");
    
    // Standard RISC-V: Reads memory before array (undefined behavior)
    // CHERI: Should trigger capability bounds violation
    int invalid_value = array[-1];
    
    printf("Value at array[-1]: %d\n", invalid_value);
    
    // Attempt negative index write
    printf("Attempting to write to array[-1]...\n");
    array[-1] = 999;
    
    printf("Negative index test completed\n");
}

int main() {
    printf("=== Off-by-One Boundary Condition Tests ===\n");
    printf("Testing Standard RISC-V vs CHERI boundary protection\n\n");
    
    // Run all off-by-one tests
    array_off_by_one_test();
    strcpy_off_by_one_test();
    loop_off_by_one_test();
    negative_index_test();
    
    printf("\n=== Test Summary ===\n");
    printf("Standard RISC-V: All off-by-one errors likely succeed, corrupting memory\n");
    printf("CHERI: Hardware bounds checking should prevent all invalid accesses\n");
    
    return 0;
}

/*
 * Expected Behavior Analysis:
 * 
 * Standard RISC-V:
 * - array[10]: Reads/writes memory after buffer (stack corruption possible)
 * - strcpy overflow: Overwrites adjacent stack variables
 * - array[5]: Reads garbage data from memory after array
 * - array[-1]: Reads memory before array (potential information disclosure)
 * 
 * Authentic CHERI:
 * - array[10]: Capability bounds exception (ca.offset >= ca.length)
 * - strcpy overflow: Bounds exception during string copy
 * - array[5]: Capability bounds exception on out-of-bounds access
 * - array[-1]: Capability bounds exception (ca.offset < ca.base)
 * 
 * Security Implications:
 * - Standard RISC-V: All boundary errors succeed, creating vulnerabilities
 * - CHERI: Hardware prevents all boundary violations, eliminating attack surface
 */
