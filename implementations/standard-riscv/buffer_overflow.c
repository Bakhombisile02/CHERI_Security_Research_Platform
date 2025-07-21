/*
 * Buffer Overflow Test - Standard RISC-V Implementation (Bare Metal)
 * 
 * This program demonstrates a classic buffer overflow vulnerability
 * in the traditional RISC-V architecture without capability protection.
 * 
 * Vulnerability: Simple strcpy() does not check destination buffer bounds,
 * allowing attacker to overflow buffer and potentially corrupt stack frame.
 */

// Simple string length function
int simple_strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Simple string copy function (vulnerable)
void simple_strcpy(char *dest, const char *src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Function demonstrating buffer overflow vulnerability
void vulnerable_function(const char *user_input) {
    char buffer[8];  // Small buffer - vulnerable to overflow
    
    // This strcpy can overflow the buffer if user_input > 8 chars
    simple_strcpy(buffer, user_input);  // VULNERABLE: No bounds checking
    
    // Buffer is now potentially overflowed
    // In Standard RISC-V: no protection, corruption possible
}

// Function to demonstrate stack corruption
void stack_canary_function() {
    unsigned long canary = 0xdeadbeefcafebabe;
    char local_buffer[16];
    
    // Initialize local buffer
    for (int i = 0; i < 16; i++) {
        local_buffer[i] = 0;
    }
    
    // Call vulnerable function with long input
    vulnerable_function("This input is definitely longer than 8 characters and will overflow the buffer");
    
    // Check if canary was corrupted (simple check without printf)
    if (canary != 0xdeadbeefcafebabe) {
        // Stack corruption detected - in real scenario this would be logged
        // For bare metal, we just note the corruption occurred
        canary = 0; // Mark corruption
    }
}

// Test function that creates overflow scenario
void test_buffer_overflow() {
    // This string is longer than 8 characters - will overflow
    const char *dangerous_input = "ThisIsAVeryLongStringThatWillOverflow";
    
    vulnerable_function(dangerous_input);
}

// Main function
int main() {
    test_buffer_overflow();
    stack_canary_function();
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
 *   call    simple_strcpy      # Unsafe copy - no bounds checking
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
