/*
 * Off-by-One Buffer Overflow Test (Bare Metal)
 * Tests boundary condition where buffer overflow occurs by exactly one byte
 */

// Simple string copy function (vulnerable)
void simple_strcpy(char *dest, const char *src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Test off-by-one overflow
void test_off_by_one_overflow() {
    char buffer[8];  // 8 byte buffer
    
    // This string is exactly 8 characters (plus null terminator = 9 bytes)
    // Will overflow by exactly 1 byte
    const char *input = "12345678";  // 8 chars + null = 9 bytes > 8 byte buffer
    
    simple_strcpy(buffer, input);  // Off-by-one overflow!
}

// Test boundary cases
void test_boundary_cases() {
    char small_buffer[4];
    
    // Exact fit (3 chars + null terminator = 4 bytes)
    simple_strcpy(small_buffer, "123");  // Safe
    
    // Off-by-one (4 chars + null terminator = 5 bytes > 4 byte buffer)
    simple_strcpy(small_buffer, "1234");  // Overflow!
}

int main() {
    test_off_by_one_overflow();
    test_boundary_cases();
    return 0;
}
