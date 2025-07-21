/*
 * Buffer Overflow Test - Standard RISC-V Implementation
 * 
 * This program demonstrates a classic buffer overflow vulnerability
 * in the traditional RISC-V architecture without capability protection.
 * 
 * Vulnerability: strcpy() does not check destination buffer bounds,
 * allowing attacker to overflow buffer and potentially corrupt stack frame.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function demonstrating buffer overflow vulnerability
void vulnerable_function(const char *user_input) {
    char buffer[8];  // Small buffer - vulnerable to overflow
    
    printf("Buffer address: %p\n", buffer);
    printf("Input length: %zu\n", strlen(user_input));
    
    // This strcpy can overflow the buffer if user_input > 8 chars
    strcpy(buffer, user_input);  // VULNERABLE: No bounds checking
    
    printf("Buffer content: %s\n", buffer);
    // Buffer is now potentially overflowed
    // In Standard RISC-V: no protection, corruption possible
}

// Function to demonstrate stack corruption
void stack_canary_function() {
    unsigned long canary = 0xdeadbeefcafebabe;
    char local_buffer[16];
    
    printf("Stack canary before: 0x%lx\n", canary);
    
    // Call vulnerable function
    vulnerable_function("This input is definitely longer than 8 characters and will overflow the buffer");
    
    printf("Stack canary after: 0x%lx\n", canary);
    
    if (canary != 0xdeadbeefcafebabe) {
        printf("SECURITY VIOLATION: Stack corruption detected!\n");
        printf("Buffer overflow has corrupted adjacent stack variables\n");
    } else {
        printf("Stack canary intact (no overflow detected)\n");
    }
}

// Test function that creates overflow scenario
void test_buffer_overflow() {
    printf("=== Testing Buffer Overflow ===\n");
    
    // This string is longer than 8 characters - will overflow
    const char *dangerous_input = "ThisIsAVeryLongStringThatWillOverflow";
    
    vulnerable_function(dangerous_input);
    printf("Function completed successfully\n");
}

int main(int argc, char *argv[]) {
    printf("=== Standard RISC-V Buffer Overflow Test ===\n");
    printf("Architecture: rv64imac (Traditional 64-bit RISC-V)\n");
    printf("Protection: None (vulnerable to buffer overflow)\n\n");
    
    if (argc < 2) {
        printf("Usage: %s <input_string>\n", argv[0]);
        printf("Try: %s \"Short\"\n", argv[0]);
        printf("Try: %s \"This_is_a_very_long_input_that_will_overflow_the_8_byte_buffer\"\n", argv[0]);
        
        // Run default test
        test_buffer_overflow();
        stack_canary_function();
        return 0;
    }
    
    // Test with user input
    printf("Testing with input: \"%s\"\n", argv[1]);
    vulnerable_function(argv[1]);
    
    return 0;
}

/*
 * Expected Assembly Output (Standard RISC-V):
 * 
 * vulnerable_function:
 *   addi    sp, sp, -32        # Allocate stack frame
 *   sd      ra, 24(sp)         # Save return address
 *   sd      s0, 16(sp)         # Save frame pointer
 *   addi    s0, sp, 32         # Set frame pointer
 *   
 *   addi    a0, s0, -16        # Calculate buffer address (no bounds info)
 *   mv      a1, a0             # Prepare for strcpy
 *   call    strcpy             # Unsafe copy - no bounds checking
 *   
 *   # No bounds verification, no protection against overflow
 *   # Buffer overflow can corrupt saved registers and return address
 * 
 * Security Analysis:
 * - Buffer allocated as simple stack offset (s0-16)
 * - No hardware bounds checking
 * - strcpy() performs unbounded memory copy
 * - Stack corruption possible if input > 8 characters
 * - Return address corruption possible for larger inputs
 * - No capability metadata to enforce spatial safety
 */
