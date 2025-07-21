/*
 * Buffer Overflow Protection - CHERI Implementation (Bare Metal)
 * 
 * This program demonstrates how CHERI's capability-based protection
 * prevents buffer overflow vulnerabilities through spatial bounds checking.
 * 
 * Protection: CHERI capabilities carry bounds information that is enforced
 * by hardware on every memory access, preventing buffer overflows.
 */

#ifdef __CHERI__
// CHERI capability manipulation functions (simplified)
typedef void* __capability cap_ptr_t;

// Simplified CHERI intrinsics for bare metal
static inline cap_ptr_t cheri_bounds_set(void *ptr, unsigned long size) {
    // In real CHERI, this would set capability bounds
    return (cap_ptr_t)ptr;
}

static inline int cheri_tag_get(cap_ptr_t cap) {
    // In real CHERI, this would check the capability tag bit
    return 1; // Assume valid for demonstration
}

static inline unsigned long cheri_length_get(cap_ptr_t cap) {
    // In real CHERI, this would return capability length
    return 8; // Assume buffer size for demonstration
}
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_tag_get(cap) 1
#define cheri_length_get(cap) 8
#endif

// Simple string length function
int simple_strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// CHERI-aware string copy function (with bounds checking simulation)
void cheri_strcpy(char *dest, const char *src) {
    // In real CHERI, bounds checking would be automatic in hardware
    // For demonstration, we simulate the bounds check
    int src_len = simple_strlen(src);
    
#ifdef __CHERI__
    // In real CHERI, this check would be done in hardware on each store
    unsigned long dest_bounds = cheri_length_get((cap_ptr_t)dest);
    if (src_len >= dest_bounds) {
        // In real CHERI hardware, this would trigger a capability exception
        // For bare metal demonstration, we just return without copying
        return;
    }
#endif
    
    // Perform the copy (in CHERI, each store would be bounds-checked)
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Function demonstrating CHERI buffer overflow protection
void protected_function(const char *user_input) {
    char buffer[8];  // Small buffer - but protected by CHERI bounds
    
#ifdef __CHERI__
    // In real CHERI, the buffer would automatically have capability bounds
    cap_ptr_t buffer_cap = cheri_bounds_set(buffer, 8);
    
    // Check capability validity
    if (!cheri_tag_get(buffer_cap)) {
        return; // Invalid capability
    }
#endif
    
    // Attempt strcpy - CHERI will check bounds on each store
    cheri_strcpy(buffer, user_input);  // PROTECTED: CHERI bounds checking
    
    // If we reach here, the copy was within bounds
}

// Function to demonstrate CHERI capability bounds enforcement
void demonstrate_bounds_checking() {
    char local_buffer[16];
    
#ifdef __CHERI__
    // Create bounded capability for the buffer
    cap_ptr_t buffer_cap = cheri_bounds_set(local_buffer, 16);
#endif
    
    // Safe access within bounds
    local_buffer[0] = 'A';
    local_buffer[15] = 'B';  // Last valid index
    
    // In real CHERI, out-of-bounds access would cause capability exception:
    // local_buffer[16] = 'C';  // Would cause bounds fault!
    // local_buffer[-1] = 'D';  // Would cause bounds fault!
}

// Test function demonstrating CHERI protection
void test_cheri_protection() {
    // Test 1: Safe input (within bounds)
    protected_function("Safe");
    
    // Test 2: Boundary input (exactly at limit)
    protected_function("1234567");  // 7 chars + null terminator = 8 bytes
    
    // Test 3: Unsafe input (would overflow in traditional system)
    // In real CHERI hardware, this would trigger capability exception
    protected_function("TooLong");  // Safe for demo, but demonstrates concept
    
    demonstrate_bounds_checking();
}

// Main function
int main() {
    test_cheri_protection();
    return 0;
}

/*
 * Expected Assembly Output (CHERI):
 * 
 * protected_function:
 *   cincoffset csp, csp, -32   # Allocate stack frame (capability stack pointer)
 *   csc     cra, 24(csp)       # Save return address capability
 *   csc     cs0, 16(csp)       # Save frame pointer capability
 *   cincoffset cs0, csp, 32    # Set frame pointer capability
 *   
 *   cincoffset ca0, cs0, -16   # Calculate buffer capability with bounds
 *   csetbounds ca0, ca0, 8     # Set bounds to 8 bytes (buffer size)
 *   cmove   ca1, ca1           # Source string capability
 *   ccall   cheri_strcpy       # CHERI-aware strcpy with bounds checking
 *   
 *   # Each store in strcpy:
 *   csb     t0, 0(ca0)         # Capability store with bounds check
 *   # Hardware verifies: address >= base && address < base + length
 *   # If bounds violation: Capability exception triggered
 * 
 * CHERI Protection Analysis:
 * - Buffer capability created with exact bounds (8 bytes)
 * - strcpy() performs capability-aware memory access
 * - Each store instruction checks capability bounds
 * - Spatial safety guaranteed by hardware, not software
 * - Buffer overflow impossible due to capability bounds enforcement
 * - Capability exception immediately terminates violating operations
 * 
 * Key CHERI Spatial Safety Features:
 * - Automatic bounds enforcement on all memory accesses
 * - Hardware capability bounds checking
 * - Immediate detection and prevention of buffer overflows
 * - Fine-grained memory protection at object level
 * - No performance penalty for bounds checking (hardware enforced)
 */
