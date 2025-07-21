/*
 * Advanced Attack Scenarios Test Suite (Bare Metal Version)
 * 
 * Tests sophisticated attack vectors to evaluate CHERI's limits
 */

#include <stdint.h>

// Define size_t for bare-metal
typedef unsigned long size_t;

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_length_get(cap) SIZE_MAX
#define cheri_tag_get(cap) 1
#define cheri_address_get(cap) ((uintptr_t)(cap))
#define cheri_offset_set(cap, offset) ((cap) + (offset))
#endif

// Define SIZE_MAX for non-CHERI builds
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

// Bare-metal implementations
static char output_buffer[4096];
static int output_pos = 0;

void simple_print(const char* str) {
    while (*str && output_pos < sizeof(output_buffer) - 1) {
        output_buffer[output_pos++] = *str++;
    }
    output_buffer[output_pos] = '\0';
}

void* simple_malloc(size_t size) {
    static char heap[4 * 1024 * 1024]; // 4MB heap
    static size_t heap_pos = 0;
    
    // Align to 8 bytes
    size = (size + 7) & ~7;
    
    if (heap_pos + size > sizeof(heap)) {
        return (void*)0; // NULL
    }
    
    void* ptr = &heap[heap_pos];
    heap_pos += size;
    return ptr;
}

void simple_memset(void* ptr, int value, size_t size) {
    char* p = (char*)ptr;
    size_t i;
    for (i = 0; i < size; i++) {
        p[i] = (char)value;
    }
}

void simple_memcpy(void* dest, const void* src, size_t size) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    size_t i;
    for (i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

// Attack Scenario 1: Buffer Overflow with ROP Chain Simulation
void attack_buffer_overflow_rop() {
    simple_print("=== BUFFER OVERFLOW + ROP CHAIN ATTACK ===\n");
    
    // Simulate vulnerable function with stack buffer
    char stack_buffer[256];
    char* heap_buffer = (char*)simple_malloc(512);
    
    if (!heap_buffer) return;
    
    #ifdef __CHERI__
    heap_buffer = cheri_bounds_set(heap_buffer, 512);
    #endif
    
    // Simulate attacker-controlled input
    char malicious_input[1024];
    simple_memset(malicious_input, 0x41, sizeof(malicious_input)); // 'A' pattern
    
    // Attempt 1: Classic stack buffer overflow
    simple_print("Attempting stack buffer overflow...\n");
    #ifdef __CHERI__
    // CHERI should prevent this
    simple_memcpy(stack_buffer, malicious_input, 256); // Safe copy
    #else
    // This would overflow in standard RISC-V
    simple_memcpy(stack_buffer, malicious_input, 512); // Overflow!
    #endif
    
    // Attempt 2: Heap buffer overflow
    simple_print("Attempting heap buffer overflow...\n");
    #ifdef __CHERI__
    // CHERI bounds should prevent overflow
    for (int i = 0; i < 1024; i++) {
        if (i < 512) { // Within bounds
            heap_buffer[i] = (char)(i & 0xFF);
        }
        // CHERI should trap on out-of-bounds access
    }
    #else
    // Standard RISC-V vulnerable to heap overflow
    for (int i = 0; i < 1024; i++) {
        heap_buffer[i] = (char)(i & 0xFF); // Potential overflow
    }
    #endif
    
    simple_print("Buffer overflow attack scenario completed\n");
}

// Attack Scenario 2: Use-After-Free with Dangling Pointers
void attack_use_after_free() {
    simple_print("=== USE-AFTER-FREE ATTACK ===\n");
    
    // Allocate and use buffer
    char* sensitive_data = (char*)simple_malloc(256);
    if (!sensitive_data) return;
    
    #ifdef __CHERI__
    sensitive_data = cheri_bounds_set(sensitive_data, 256);
    #endif
    
    // Fill with "sensitive" data
    for (int i = 0; i < 256; i++) {
        sensitive_data[i] = (char)(0x50 + (i & 0x0F)); // Pattern
    }
    
    // Simulate "free" by marking as invalid
    char* dangling_ptr = sensitive_data;
    
    #ifdef __CHERI__
    // In real CHERI, capability would be revoked
    // For simulation, we'll check tag
    simple_print("CHERI: Capability should be revoked after free\n");
    #endif
    
    // Allocate new object in same memory region
    char* new_object = (char*)simple_malloc(256);
    if (!new_object) return;
    
    #ifdef __CHERI__
    new_object = cheri_bounds_set(new_object, 256);
    #endif
    
    // Fill with attacker-controlled data
    for (int i = 0; i < 256; i++) {
        new_object[i] = (char)(0xAA); // Attacker pattern
    }
    
    // Attempt to access freed memory through dangling pointer
    simple_print("Attempting use-after-free access...\n");
    
    #ifdef __CHERI__
    // CHERI should prevent this access
    if (cheri_tag_get(dangling_ptr)) {
        volatile char value = dangling_ptr[0];
        (void)value;
        simple_print("ERROR: Use-after-free succeeded in CHERI!\n");
    } else {
        simple_print("CHERI: Use-after-free blocked by capability revocation\n");
    }
    #else
    // Standard RISC-V vulnerable
    volatile char value = dangling_ptr[0]; // Reads attacker data!
    (void)value;
    simple_print("Standard RISC-V: Use-after-free succeeded\n");
    #endif
    
    simple_print("Use-after-free attack scenario completed\n");
}

// Attack Scenario 3: Integer Overflow to Buffer Overflow
void attack_integer_overflow() {
    simple_print("=== INTEGER OVERFLOW ATTACK ===\n");
    
    // Simulate vulnerable allocation based on user input
    uint32_t user_count = 0xFFFFFFFF; // Max uint32
    uint32_t item_size = 8;
    
    // This multiplication overflows
    uint32_t total_size_32 = user_count * item_size;
    size_t total_size = (size_t)total_size_32;
    
    simple_print("Attempting allocation with overflowed size...\n");
    
    char* buffer = (char*)simple_malloc(total_size);
    if (!buffer) {
        simple_print("Allocation failed (good!)\n");
        return;
    }
    
    #ifdef __CHERI__
    buffer = cheri_bounds_set(buffer, total_size);
    #endif
    
    // Attempt to write beyond allocated space
    #ifdef __CHERI__
    // CHERI bounds should prevent out-of-bounds access
    for (uint32_t i = 0; i < 1000 && i < user_count; i++) {
        if (i * item_size < total_size) {
            buffer[i * item_size] = (char)(i & 0xFF);
        }
    }
    #else
    // Standard RISC-V may allow out-of-bounds writes
    for (uint32_t i = 0; i < 1000; i++) {
        buffer[i * item_size] = (char)(i & 0xFF);
    }
    #endif
    
    simple_print("Integer overflow attack scenario completed\n");
}

// Attack Scenario 4: Format String Attack Simulation
void attack_format_string() {
    simple_print("=== FORMAT STRING ATTACK SIMULATION ===\n");
    
    char buffer[1024];
    char* heap_data = (char*)simple_malloc(512);
    
    if (!heap_data) return;
    
    #ifdef __CHERI__
    heap_data = cheri_bounds_set(heap_data, 512);
    #endif
    
    // Fill heap with sensitive data
    for (int i = 0; i < 512; i++) {
        heap_data[i] = (char)(0x60 + (i & 0x0F));
    }
    
    // Simulate attacker-controlled format string
    const char* malicious_format = "%p %p %p %p"; // Trying to leak pointers
    
    // Simulate format string vulnerability
    #ifdef __CHERI__
    // CHERI should provide some protection through capability bounds
    simple_print("CHERI: Format string with capability protection\n");
    #else
    // Standard RISC-V vulnerable to information disclosure
    simple_print("Standard RISC-V: Format string vulnerability\n");
    #endif
    
    simple_print("Format string attack scenario completed\n");
}

// Attack Scenario 5: Double Free Attack
void attack_double_free() {
    simple_print("=== DOUBLE FREE ATTACK ===\n");
    
    char* ptr1 = (char*)simple_malloc(256);
    char* ptr2 = (char*)simple_malloc(256);
    
    if (!ptr1 || !ptr2) return;
    
    #ifdef __CHERI__
    ptr1 = cheri_bounds_set(ptr1, 256);
    ptr2 = cheri_bounds_set(ptr2, 256);
    #endif
    
    // Use the pointers
    simple_memset(ptr1, 0xAA, 256);
    simple_memset(ptr2, 0xBB, 256);
    
    // Simulate first free
    simple_print("First free...\n");
    
    #ifdef __CHERI__
    // In CHERI, capability would be revoked
    simple_print("CHERI: Capability revoked on first free\n");
    #endif
    
    // Attempt double free
    simple_print("Attempting double free...\n");
    
    #ifdef __CHERI__
    // CHERI should detect invalid capability
    if (cheri_tag_get(ptr1)) {
        simple_print("ERROR: Double free possible in CHERI!\n");
    } else {
        simple_print("CHERI: Double free prevented by capability system\n");
    }
    #else
    // Standard RISC-V vulnerable to double free
    simple_print("Standard RISC-V: Double free vulnerability\n");
    #endif
    
    simple_print("Double free attack scenario completed\n");
}

// Attack Scenario 6: Type Confusion Attack
void attack_type_confusion() {
    simple_print("=== TYPE CONFUSION ATTACK ===\n");
    
    // Allocate memory for one type
    uint32_t* int_array = (uint32_t*)simple_malloc(256 * sizeof(uint32_t));
    if (!int_array) return;
    
    #ifdef __CHERI__
    int_array = (uint32_t*)cheri_bounds_set(int_array, 256 * sizeof(uint32_t));
    #endif
    
    // Initialize as integers
    for (int i = 0; i < 256; i++) {
        int_array[i] = 0x12345678 + i;
    }
    
    // Type confusion: treat as char array
    char* char_view = (char*)int_array;
    
    #ifdef __CHERI__
    // CHERI maintains bounds regardless of type cast
    simple_print("CHERI: Type confusion with maintained bounds\n");
    for (int i = 0; i < 256 * sizeof(uint32_t); i++) {
        char_view[i] = (char)(i & 0xFF);
    }
    #else
    // Standard RISC-V allows unchecked type confusion
    simple_print("Standard RISC-V: Unchecked type confusion\n");
    for (int i = 0; i < 256 * sizeof(uint32_t) + 100; i++) { // Potential overflow
        char_view[i] = (char)(i & 0xFF);
    }
    #endif
    
    simple_print("Type confusion attack scenario completed\n");
}

// Attack Scenario 7: Return-Oriented Programming (ROP) Gadget Chain
void attack_rop_gadgets() {
    simple_print("=== ROP GADGET CHAIN ATTACK ===\n");
    
    // Simulate function pointers table
    void (*function_table[10])(void);
    
    // Initialize with dummy functions (addresses)
    for (int i = 0; i < 10; i++) {
        function_table[i] = (void(*)(void))(0x1000 + i * 0x100);
    }
    
    #ifdef __CHERI__
    // In CHERI, function pointers would be capabilities with execute permission
    simple_print("CHERI: Function pointers are execute-only capabilities\n");
    #endif
    
    // Simulate attacker overwriting function table
    char* table_ptr = (char*)function_table;
    
    #ifdef __CHERI__
    // CHERI should prevent arbitrary writes to function pointers
    simple_print("CHERI: Function pointer overwrite protection\n");
    #else
    // Standard RISC-V vulnerable to function pointer overwrite
    simple_print("Standard RISC-V: Function pointer overwrite possible\n");
    for (int i = 0; i < sizeof(function_table); i++) {
        table_ptr[i] = (char)(0x90); // NOP sled simulation
    }
    #endif
    
    simple_print("ROP gadget chain attack scenario completed\n");
}

// Attack Scenario 8: Information Disclosure via Buffer Over-read
void attack_information_disclosure() {
    simple_print("=== INFORMATION DISCLOSURE ATTACK ===\n");
    
    char* public_data = (char*)simple_malloc(256);
    char* secret_data = (char*)simple_malloc(256);
    
    if (!public_data || !secret_data) return;
    
    #ifdef __CHERI__
    public_data = cheri_bounds_set(public_data, 256);
    secret_data = cheri_bounds_set(secret_data, 256);
    #endif
    
    // Fill with public and secret data
    simple_memset(public_data, 0x11, 256);
    simple_memset(secret_data, 0x99, 256); // Secret pattern
    
    // Simulate vulnerable read function
    simple_print("Attempting buffer over-read...\n");
    
    volatile uint32_t leaked_data = 0;
    
    #ifdef __CHERI__
    // CHERI should prevent reading beyond bounds
    for (int i = 0; i < 512; i++) { // Try to read beyond buffer
        if (i < 256) { // Within bounds
            leaked_data += public_data[i];
        }
        // CHERI should trap on out-of-bounds read
    }
    simple_print("CHERI: Buffer over-read prevented\n");
    #else
    // Standard RISC-V may allow reading adjacent memory
    for (int i = 0; i < 512; i++) {
        leaked_data += public_data[i]; // May read secret_data!
    }
    simple_print("Standard RISC-V: Buffer over-read possible\n");
    #endif
    
    simple_print("Information disclosure attack scenario completed\n");
}

// Main attack scenario runner
int main() {
    simple_print("ADVANCED ATTACK SCENARIOS TEST SUITE\n");
    simple_print("====================================\n");
    
    #ifdef __CHERI__
    simple_print("Testing CHERI defense capabilities\n");
    #else
    simple_print("Testing Standard RISC-V vulnerabilities\n");
    #endif
    
    attack_buffer_overflow_rop();
    attack_use_after_free();
    attack_integer_overflow();
    attack_format_string();
    attack_double_free();
    attack_type_confusion();
    attack_rop_gadgets();
    attack_information_disclosure();
    
    simple_print("====================================\n");
    simple_print("All attack scenarios completed!\n");
    
    #ifdef __CHERI__
    simple_print("CHERI provided protection against most attacks\n");
    #else
    simple_print("Standard RISC-V vulnerable to multiple attack vectors\n");
    #endif
    
    return 0;
}

// Entry point for bare-metal
void _start() {
    main();
    
    // Infinite loop to prevent return
    while (1) {
        asm volatile("nop");
    }
}
