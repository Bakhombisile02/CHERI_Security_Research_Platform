/*
 * Use-After-Free Protection - CHERI Implementation (Bare Metal)
 * 
 * This program demonstrates temporal memory safety in CHERI architecture.
 * CHERI capabilities provide temporal protection - when memory is freed,
 * capabilities become invalid and cannot be used to access the memory.
 */

#ifdef __CHERI__
// CHERI capability types and functions
typedef void* __capability cap_ptr_t;

// Simplified CHERI capability functions for bare metal
static inline cap_ptr_t cheri_bounds_set(void *ptr, unsigned long size) {
    return (cap_ptr_t)ptr;
}

static inline int cheri_tag_get(cap_ptr_t cap) {
    // In real CHERI, this would check the capability tag bit
    return 1; // Assume valid initially
}

static inline void cheri_tag_clear(cap_ptr_t *cap) {
    // In real CHERI, this would clear the capability tag bit
    // For demonstration, we simulate by setting to null
    *cap = (cap_ptr_t)0;
}
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_tag_get(cap) 1
#define cheri_tag_clear(cap) do { *(cap) = 0; } while(0)
#endif

// Simple memory allocation simulation with capability tracking
static char memory_pool[1024];
static int next_alloc = 0;
static cap_ptr_t allocated_caps[32]; // Track allocated capabilities
static int num_caps = 0;

// CHERI-aware malloc simulation
cap_ptr_t cheri_malloc(int size) {
    if (next_alloc + size >= 1024) return (cap_ptr_t)0; // Out of memory
    
    void* ptr = &memory_pool[next_alloc];
    next_alloc += size;
    
    // Create a bounded capability for the allocated region
    cap_ptr_t cap = cheri_bounds_set(ptr, size);
    
    // Track this capability
    if (num_caps < 32) {
        allocated_caps[num_caps] = cap;
        num_caps++;
    }
    
    return cap;
}

// CHERI-aware free simulation that invalidates capabilities
void cheri_free(cap_ptr_t ptr) {
    if (ptr == (cap_ptr_t)0) return;
    
    // Find and invalidate this capability
    for (int i = 0; i < num_caps; i++) {
        if (allocated_caps[i] == ptr) {
            // In real CHERI implementation, the capability would be invalidated
            // by the memory allocator, making it impossible to dereference
            cheri_tag_clear(&allocated_caps[i]);
            break;
        }
    }
    
    // In real CHERI, all copies of this capability would be invalidated
}

// Function demonstrating CHERI temporal protection
void protected_use_after_free() {
    // Allocate some memory with CHERI capability
    cap_ptr_t buffer = cheri_malloc(32);
    
    if (buffer == (cap_ptr_t)0) return;
    
    // Write some data (this works - capability is valid)
    char* buf_ptr = (char*)buffer;
    for (int i = 0; i < 10; i++) {
        buf_ptr[i] = 'A' + i;
    }
    buf_ptr[10] = '\0';
    
    // Free the memory (invalidates the capability)
    cheri_free(buffer);
    
    // ATTEMPT: Use after free
    // In CHERI: This should cause a capability fault!
    // The capability is no longer valid for accessing this memory
    
    if (!cheri_tag_get(buffer)) {
        // Capability has been invalidated - CHERI protection working!
        return;
    }
    
    // This access would cause a capability exception in real CHERI
    // buf_ptr[0] = 'X';  // Capability fault!
}

// Function demonstrating CHERI double-free protection
void protected_double_free() {
    cap_ptr_t buffer = cheri_malloc(16);
    
    if (buffer == (cap_ptr_t)0) return;
    
    // Use the buffer
    char* buf_ptr = (char*)buffer;
    buf_ptr[0] = 'Z';
    
    // Free it once
    cheri_free(buffer);
    
    // ATTEMPT: Double free
    // In CHERI: The capability should be invalid, preventing double-free
    cheri_free(buffer);  // Safe in CHERI - invalid capability
}

// Demonstration of CHERI bounds checking
void demonstrate_bounds_protection() {
    cap_ptr_t buffer = cheri_malloc(16);
    
    if (buffer == (cap_ptr_t)0) return;
    
    char* buf_ptr = (char*)buffer;
    
    // Valid access within bounds
    buf_ptr[0] = 'A';
    buf_ptr[15] = 'B';  // Last valid index
    
    // Invalid access - would cause bounds fault in CHERI
    // buf_ptr[16] = 'C';  // Bounds fault!
    // buf_ptr[-1] = 'D';  // Bounds fault!
    
    cheri_free(buffer);
}

// Test function
void test_cheri_protection() {
    protected_use_after_free();
    protected_double_free();
    demonstrate_bounds_protection();
}

// Main function
int main() {
    test_cheri_protection();
    return 0;
}
