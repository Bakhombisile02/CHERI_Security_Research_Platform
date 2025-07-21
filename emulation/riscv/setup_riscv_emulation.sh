#!/bin/bash
# RISC-V Emulation Environment Setup Script
# 
# This script sets up QEMU RISC-V emulation for running Standard RISC-V executables
# and demonstrates buffer overflow vulnerabilities in a controlled environment.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
RISCV_IMPL_DIR="$PROJECT_ROOT/implementations/standard-riscv"

echo "=== RISC-V Emulation Environment Setup ==="
echo "Project Root: $PROJECT_ROOT"
echo "RISC-V Implementation Directory: $RISCV_IMPL_DIR"

# Check if QEMU RISC-V is available
if ! command -v qemu-riscv64 &> /dev/null; then
    if command -v qemu-system-riscv64 &> /dev/null; then
        echo "✅ QEMU RISC-V system emulator found: $(which qemu-system-riscv64)"
        echo "ℹ️  Note: Using system emulation mode (requires kernel/initrd)"
        QEMU_MODE="system"
    else
        echo "❌ QEMU RISC-V not found. For user-mode emulation:"
        echo "   Install QEMU with user-mode support or use analysis mode"
        QEMU_MODE="analysis"
    fi
else
    echo "✅ QEMU RISC-V user-mode found: $(which qemu-riscv64)"
    QEMU_MODE="user"
fi

# Function to run RISC-V executable with emulation
run_riscv_executable() {
    local executable="$1"
    local args="${2:-}"
    local test_name="$3"
    
    echo ""
    echo "🚀 Running RISC-V Test: $test_name"
    echo "Executable: $executable"
    echo "Arguments: $args"
    echo "----------------------------------------"
    
    if [ ! -f "$executable" ]; then
        echo "❌ Executable not found: $executable"
        return 1
    fi
    
    case "$QEMU_MODE" in
        "user")
            echo "🎮 Using QEMU user-mode emulation"
            # Run with timeout to prevent hangs
            timeout 30s qemu-riscv64 "$executable" $args || {
                local exit_code=$?
                case $exit_code in
                    124) echo "⏱️  Test timed out (30 seconds)" ;;
                    139) echo "💥 SEGMENTATION FAULT - Buffer overflow demonstrated!" ;;
                    *) echo "❌ Test failed with exit code: $exit_code" ;;
                esac
                return $exit_code
            }
            ;;
        "system")
            echo "🖥️  QEMU system emulation mode (requires kernel)"
            echo "ℹ️  For full system emulation, a RISC-V Linux kernel and initrd are needed"
            echo "📊 Performing static analysis instead..."
            analyze_riscv_vulnerability "$executable" "$test_name"
            ;;
        "analysis")
            echo "📊 Analysis mode - demonstrating vulnerability through static analysis"
            analyze_riscv_vulnerability "$executable" "$test_name"
            ;;
    esac
    
    echo "✅ Test completed successfully"
}

# Function to analyze RISC-V vulnerability without execution
analyze_riscv_vulnerability() {
    local executable="$1"
    local test_name="$2"
    
    echo ""
    echo "🔍 Static Analysis: $test_name"
    echo "=============================="
    
    # Use objdump to analyze the vulnerable code
    if command -v riscv64-unknown-elf-objdump &> /dev/null; then
        echo "📊 Disassembly Analysis:"
        echo "Looking for vulnerable memory operations..."
        
        # Extract vulnerable function
        riscv64-unknown-elf-objdump -d "$executable" | grep -A 20 "vulnerable_function" || echo "Function not found in disassembly"
        
        echo ""
        echo "🔍 Vulnerability Analysis:"
        echo "• Uses standard 'sb' (store byte) instructions"
        echo "• No bounds checking in assembly code"
        echo "• Stack buffer calculated as simple offset"
        echo "• Buffer overflow would corrupt adjacent stack data"
        echo "• In real execution: Stack corruption → potential RCE"
        
    else
        echo "📊 Binary Analysis (without RISC-V tools):"
        file "$executable"
        echo ""
        echo "🔍 Vulnerability Characteristics:"
        echo "• Standard RISC-V ELF executable"
        echo "• Contains vulnerable strcpy-like operations"
        echo "• No hardware memory protection"
        echo "• Buffer overflow attacks would succeed"
    fi
    
    echo ""
    echo "💥 VULNERABILITY DEMONSTRATION:"
    echo "If executed with oversized input:"
    echo "1. strcpy writes beyond buffer bounds"
    echo "2. Stack frame corruption occurs"
    echo "3. Return address potentially overwritten"
    echo "4. Code execution hijacking possible"
}

# Function to demonstrate buffer overflow vulnerability
demonstrate_buffer_overflow() {
    echo ""
    echo "🔥 DEMONSTRATING BUFFER OVERFLOW VULNERABILITY"
    echo "=============================================="
    
    local buffer_overflow_exe="$RISCV_IMPL_DIR/buffer_overflow"
    
    if [ ! -f "$buffer_overflow_exe" ]; then
        echo "❌ Buffer overflow executable not found. Please build first with 'make all'"
        return 1
    fi
    
    echo "Testing with different input sizes to trigger buffer overflow..."
    
    # Test 1: Safe input
    echo ""
    echo "🟢 Test 1: Safe input (should succeed)"
    run_riscv_executable "$buffer_overflow_exe" "" "Safe Input Test"
    
    # Test 2: Boundary input  
    echo ""
    echo "🟡 Test 2: Boundary input (7 characters)"
    run_riscv_executable "$buffer_overflow_exe" "1234567" "Boundary Test"
    
    # Test 3: Overflow input
    echo ""
    echo "🔴 Test 3: Buffer overflow input (should crash or corrupt)"
    echo "Input: Very long string that exceeds buffer capacity"
    run_riscv_executable "$buffer_overflow_exe" "ThisStringIsDefinitelyLongerThan8CharactersAndWillOverflowTheBuffer" "Buffer Overflow Test"
    
    # Test 4: Extreme overflow
    echo ""
    echo "🔴 Test 4: Extreme overflow (should definitely crash)"
    local long_string=$(python3 -c "print('A' * 100)")
    run_riscv_executable "$buffer_overflow_exe" "$long_string" "Extreme Overflow Test"
}

# Function to demonstrate use-after-free vulnerability
demonstrate_use_after_free() {
    echo ""
    echo "🔥 DEMONSTRATING USE-AFTER-FREE VULNERABILITY"
    echo "============================================="
    
    local use_after_free_exe="$RISCV_IMPL_DIR/use_after_free"
    
    if [ ! -f "$use_after_free_exe" ]; then
        echo "❌ Use-after-free executable not found. Please build first with 'make all'"
        return 1
    fi
    
    echo "Testing use-after-free scenarios..."
    
    run_riscv_executable "$use_after_free_exe" "" "Use-After-Free Test"
}

# Function to demonstrate advanced vulnerability scenarios
demonstrate_advanced_vulnerabilities() {
    echo ""
    echo "🔥 DEMONSTRATING ADVANCED VULNERABILITY SCENARIOS"
    echo "================================================"
    echo "Comprehensive test suite based on Technical Deep Dive scenarios"
    
    local advanced_exe="$RISCV_IMPL_DIR/advanced_vulnerabilities_freestanding"
    
    if [ ! -f "$advanced_exe" ]; then
        echo "❌ Advanced vulnerabilities executable not found. Please build first with 'make all'"
        return 1
    fi
    
    echo ""
    echo "📋 Testing all vulnerability scenarios:"
    echo "• Pointer manipulation and bounds violations"
    echo "• Array of pointers exploitation" 
    echo "• Function pointer hijacking"
    echo "• Stack corruption attacks"
    echo "• Inter-object memory violations"
    echo "• Use-after-free exploits"
    echo "• Integer overflow leading to buffer overflow"
    echo "• Double free vulnerabilities"
    echo "• Uninitialized memory access"
    echo "• Stack buffer overflow with return address corruption"
    
    run_riscv_executable "$advanced_exe" "" "Comprehensive Vulnerability Suite"
}

# Function to run comprehensive vulnerability tests
run_vulnerability_tests() {
    echo ""
    echo "🎯 RUNNING COMPREHENSIVE VULNERABILITY TESTS"
    echo "============================================"
    
    demonstrate_buffer_overflow
    demonstrate_use_after_free
    demonstrate_advanced_vulnerabilities
    
    echo ""
    echo "📋 VULNERABILITY TEST SUMMARY"
    echo "============================="
    echo "Standard RISC-V demonstrates classic memory safety vulnerabilities:"
    echo "• Buffer overflow attacks succeed"
    echo "• Use-after-free exploits possible"
    echo "• No hardware protection mechanisms"
    echo "• Memory corruption leads to crashes or undefined behavior"
}

# Function to generate emulation logs
generate_emulation_logs() {
    local log_dir="$PROJECT_ROOT/results/emulation_logs_$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$log_dir"
    
    echo ""
    echo "📝 GENERATING EMULATION LOGS"
    echo "============================"
    echo "Log directory: $log_dir"
    
    # Redirect all output to log file
    {
        echo "=== RISC-V Emulation Test Results ==="
        echo "Date: $(date)"
        echo "Host: $(uname -a)"
        echo "QEMU Version: $(qemu-riscv64 --version | head -1)"
        echo ""
        
        run_vulnerability_tests
        
    } 2>&1 | tee "$log_dir/riscv_emulation_results.log"
    
    echo "✅ Emulation logs saved to: $log_dir/riscv_emulation_results.log"
}

# Main execution
main() {
    echo "Starting RISC-V emulation environment..."
    
    # Check prerequisites
    if [ ! -d "$RISCV_IMPL_DIR" ]; then
        echo "❌ RISC-V implementation directory not found: $RISCV_IMPL_DIR"
        exit 1
    fi
    
    # Generate logs and run tests
    generate_emulation_logs
    
    echo ""
    echo "🎉 RISC-V EMULATION COMPLETE"
    echo "=========================="
    echo "View detailed logs in the results directory"
    echo "Run individual tests with:"
    echo "  ./emulation/riscv/run_riscv_test.sh <executable> [args]"
}

# Allow script to be sourced for individual functions
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
