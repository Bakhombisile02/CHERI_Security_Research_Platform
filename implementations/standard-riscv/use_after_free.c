/*
 * Use-After-Free Test - Standard RISC-V Implementation (Bare Metal)
 * 
 * This program demonstrates a classic use-after-free vulnerability
 * in the traditional RISC-V architecture without capability protection.
 * 
 * Vulnerability: Using freed memory pointers can lead to undefined behavior
 * and potential security exploits.
 */

// Simple memory allocation simulation (for demonstration)
static char memory_pool[1024];
static int next_alloc = 0;

// Simple malloc simulation
void* simple_malloc(int size) {
    if (next_alloc + size >= 1024) return 0; // Out of memory
    void* ptr = &memory_pool[next_alloc];
    next_alloc += size;
    return ptr;
}

// Simple free simulation (doesn't actually free memory in this demo)
void simple_free(void* ptr) {
    // In real implementation, this would mark memory as free
    // For demo purposes, we just leave the memory accessible
    // This creates the use-after-free vulnerability
    (void)ptr; // Suppress unused parameter warning
}

// Function demonstrating use-after-free vulnerability
void vulnerable_use_after_free() {
    // Allocate some memory
    char* buffer = (char*)simple_malloc(32);
    
    if (buffer == 0) return; // Allocation failed
    
    // Write some data
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A' + i;
    }
    buffer[10] = '\0';
    
    // Free the memory
    simple_free(buffer);
    
    // VULNERABILITY: Use after free
    // In Standard RISC-V: no protection, can access freed memory
    buffer[0] = 'X';  // This should not be allowed!
    buffer[5] = 'Y';  // Accessing freed memory
}

// Function demonstrating double-free vulnerability
void vulnerable_double_free() {
    char* buffer = (char*)simple_malloc(16);
    
    if (buffer == 0) return; // Allocation failed
    
    // Use the buffer
    buffer[0] = 'Z';
    
    // Free it once
    simple_free(buffer);
    
    // VULNERABILITY: Double free
    simple_free(buffer);  // Freeing already freed memory
}

// Test function
void test_use_after_free() {
    vulnerable_use_after_free();
    vulnerable_double_free();
}

// Main function
int main() {
    test_use_after_free();
    return 0;
}
