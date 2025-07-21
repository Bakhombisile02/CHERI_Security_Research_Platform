# CHERI Security Research Platform - Makefile
# Comprehensive build system for Standard RISC-V vs Authentic CHERI comparison

# Toolchain Configuration
RISCV_CC = riscv64-elf-gcc
CHERI_CC = /Users/dlaba556/cheri/output/sdk/bin/clang

# Architecture Targets
RISCV_ARCH = rv64imac
RISCV_ABI = lp64
CHERI_CONFIG = cheribsd-riscv64-purecap

# Build Directories
RISCV_DIR = implementations/standard-riscv
CHERI_DIR = implementations/authentic-cheri
ANALYSIS_DIR = comparative-analysis
RESULTS_DIR = results
RAW_OUTPUTS_DIR = extreme-details/raw-outputs

# Compilation Flags
RISCV_CFLAGS = -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -nostdlib -nostartfiles -ffreestanding -O2 -g3 -Wall -Wextra
CHERI_CFLAGS = --config $(CHERI_CONFIG) -nostdlib -nostartfiles -ffreestanding -O2 -g3 -Wall -Wextra

# Assembly Generation Flags
RISCV_ASMFLAGS = -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -nostdlib -nostartfiles -ffreestanding -S -fverbose-asm
CHERI_ASMFLAGS = --config $(CHERI_CONFIG) -nostdlib -nostartfiles -ffreestanding -S -fverbose-asm

# Test Programs (Standard RISC-V)
PROGRAMS = buffer_overflow use_after_free advanced_vulnerabilities advanced_vulnerabilities_freestanding

# CHERI Test Programs 
CHERI_PROGRAMS = buffer_overflow_cheri use_after_free_cheri advanced_protections_cheri advanced_protections_cheri_freestanding

# Edge Case Test Programs
EDGE_CASES_DIR = extreme-details/edge-cases
BOUNDARY_TESTS = test-off-by-one test-zero-length-buffers
CORNER_TESTS = test-negative-indices test-integer-overflow
STRESS_TESTS = test-recursive-calls

# Stress Testing Programs
STRESS_TESTING_DIR = extreme-details/stress-testing
STRESS_PROGRAMS = cheri-stress-tests standard-riscv-stress-tests cheri-failure-points real-world-network-stress

# Default target
.PHONY: all clean analyze compare setup compile-edge-cases compile-stress-tests

all: setup compile-all compile-edge-cases compile-stress-tests analyze

# Setup directories
setup:
	@echo "Setting up CHERI Security Research Platform..."
	@mkdir -p $(RESULTS_DIR)
	@mkdir -p $(RAW_OUTPUTS_DIR)/standard-riscv
	@mkdir -p $(RAW_OUTPUTS_DIR)/authentic-cheri
	@mkdir -p $(RAW_OUTPUTS_DIR)/comparison-data
	@echo "Directory structure created."

# Compile all programs for both architectures
compile-all: compile-riscv compile-cheri generate-assembly

# Compile edge case tests for both architectures
compile-edge-cases: compile-edge-riscv compile-edge-cheri

# Standard RISC-V edge case compilation
compile-edge-riscv:
	@echo "Compiling edge case tests for Standard RISC-V..."
	@mkdir -p $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases
	@for test in $(BOUNDARY_TESTS); do \
		echo "Compiling boundary test: $$test"; \
		$(RISCV_CC) $(RISCV_CFLAGS) $(EDGE_CASES_DIR)/boundary-conditions/$$test.c \
			-o $(EDGE_CASES_DIR)/boundary-conditions/$$test\_riscv \
			2>&1 | tee $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases/$$test\_compilation.log; \
		$(RISCV_CC) $(RISCV_ASMFLAGS) $(EDGE_CASES_DIR)/boundary-conditions/$$test.c \
			-o $(EDGE_CASES_DIR)/boundary-conditions/$$test\_riscv.s \
			2>&1 | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases/$$test\_assembly.log; \
	done
	@for test in $(CORNER_TESTS); do \
		echo "Compiling corner case test: $$test"; \
		$(RISCV_CC) $(RISCV_CFLAGS) $(EDGE_CASES_DIR)/corner-cases/$$test.c \
			-o $(EDGE_CASES_DIR)/corner-cases/$$test\_riscv \
			2>&1 | tee $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases/$$test\_compilation.log; \
		$(RISCV_CC) $(RISCV_ASMFLAGS) $(EDGE_CASES_DIR)/corner-cases/$$test.c \
			-o $(EDGE_CASES_DIR)/corner-cases/$$test\_riscv.s \
			2>&1 | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases/$$test\_assembly.log; \
	done
	@for test in $(STRESS_TESTS); do \
		echo "Compiling stress test: $$test"; \
		$(RISCV_CC) $(RISCV_CFLAGS) $(EDGE_CASES_DIR)/stress-tests/$$test.c \
			-o $(EDGE_CASES_DIR)/stress-tests/$$test\_riscv \
			2>&1 | tee $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases/$$test\_compilation.log; \
		$(RISCV_CC) $(RISCV_ASMFLAGS) $(EDGE_CASES_DIR)/stress-tests/$$test.c \
			-o $(EDGE_CASES_DIR)/stress-tests/$$test\_riscv.s \
			2>&1 | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/edge-cases/$$test\_assembly.log; \
	done

# Authentic CHERI edge case compilation
compile-edge-cheri:
	@echo "Compiling edge case tests for Authentic CHERI..."
	@mkdir -p $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases
	@for test in $(BOUNDARY_TESTS); do \
		echo "Compiling CHERI boundary test: $$test"; \
		$(CHERI_CC) $(CHERI_CFLAGS) $(EDGE_CASES_DIR)/boundary-conditions/$$test.c \
			-o $(EDGE_CASES_DIR)/boundary-conditions/$$test\_cheri \
			2>&1 | tee $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases/$$test\_compilation.log; \
		$(CHERI_CC) $(CHERI_ASMFLAGS) $(EDGE_CASES_DIR)/boundary-conditions/$$test.c \
			-o $(EDGE_CASES_DIR)/boundary-conditions/$$test\_cheri.s \
			2>&1 | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases/$$test\_assembly.log; \
	done
	@for test in $(CORNER_TESTS); do \
		echo "Compiling CHERI corner case test: $$test"; \
		$(CHERI_CC) $(CHERI_CFLAGS) $(EDGE_CASES_DIR)/corner-cases/$$test.c \
			-o $(EDGE_CASES_DIR)/corner-cases/$$test\_cheri \
			2>&1 | tee $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases/$$test\_compilation.log; \
		$(CHERI_CC) $(CHERI_ASMFLAGS) $(EDGE_CASES_DIR)/corner-cases/$$test.c \
			-o $(EDGE_CASES_DIR)/corner-cases/$$test\_cheri.s \
			2>&1 | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases/$$test\_assembly.log; \
	done
	@for test in $(STRESS_TESTS); do \
		echo "Compiling CHERI stress test: $$test"; \
		$(CHERI_CC) $(CHERI_CFLAGS) $(EDGE_CASES_DIR)/stress-tests/$$test.c \
			-o $(EDGE_CASES_DIR)/stress-tests/$$test\_cheri \
			2>&1 | tee $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases/$$test\_compilation.log; \
		$(CHERI_CC) $(CHERI_ASMFLAGS) $(EDGE_CASES_DIR)/stress-tests/$$test.c \
			-o $(EDGE_CASES_DIR)/stress-tests/$$test\_cheri.s \
			2>&1 | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/edge-cases/$$test\_assembly.log; \
	done

# Compile stress tests for both architectures
compile-stress-tests: compile-stress-riscv compile-stress-cheri

# Standard RISC-V stress test compilation
compile-stress-riscv:
	@echo "Compiling stress tests for Standard RISC-V..."
	@mkdir -p $(RAW_OUTPUTS_DIR)/standard-riscv/stress-tests
	@for test in $(STRESS_PROGRAMS); do \
		if [ -f $(STRESS_TESTING_DIR)/$$test.c ]; then \
			echo "Compiling stress test: $$test (RISC-V)"; \
			$(RISCV_CC) $(RISCV_CFLAGS) $(STRESS_TESTING_DIR)/$$test.c \
				-o $(STRESS_TESTING_DIR)/$$test\_riscv \
				2>&1 | tee $(RAW_OUTPUTS_DIR)/standard-riscv/stress-tests/$$test\_compilation.log; \
			$(RISCV_CC) $(RISCV_ASMFLAGS) $(STRESS_TESTING_DIR)/$$test.c \
				-o $(STRESS_TESTING_DIR)/$$test\_riscv.s \
				2>&1 | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/stress-tests/$$test\_assembly.log; \
		fi; \
	done

# CHERI stress test compilation
compile-stress-cheri:
	@echo "Compiling stress tests for CHERI..."
	@mkdir -p $(RAW_OUTPUTS_DIR)/authentic-cheri/stress-tests
	@for test in $(STRESS_PROGRAMS); do \
		if [ -f $(STRESS_TESTING_DIR)/$$test.c ]; then \
			echo "Compiling stress test: $$test (CHERI)"; \
			$(CHERI_CC) $(CHERI_CFLAGS) $(STRESS_TESTING_DIR)/$$test.c \
				-o $(STRESS_TESTING_DIR)/$$test\_cheri \
				2>&1 | tee $(RAW_OUTPUTS_DIR)/authentic-cheri/stress-tests/$$test\_compilation.log; \
			$(CHERI_CC) $(CHERI_ASMFLAGS) $(STRESS_TESTING_DIR)/$$test.c \
				-o $(STRESS_TESTING_DIR)/$$test\_cheri.s \
				2>&1 | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/stress-tests/$$test\_assembly.log; \
		fi; \
	done

# Standard RISC-V compilation
compile-riscv:
	@echo "Compiling Standard RISC-V implementations..."
	@for prog in $(PROGRAMS); do \
		if [ -f $(RISCV_DIR)/$$prog.c ]; then \
			echo "Building $$prog for Standard RISC-V..."; \
			$(RISCV_CC) $(RISCV_CFLAGS) -o $(RISCV_DIR)/$$prog $(RISCV_DIR)/$$prog.c 2>&1 | \
				tee $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_compilation.log; \
		fi; \
	done

# Authentic CHERI compilation  
compile-cheri:
	@echo "Compiling Authentic CHERI implementations..."
	@for prog in $(PROGRAMS); do \
		if [ -f $(CHERI_DIR)/$$prog\_cheri.c ]; then \
			echo "Building $$prog for Authentic CHERI..."; \
			$(CHERI_CC) $(CHERI_CFLAGS) -o $(CHERI_DIR)/$$prog\_cheri $(CHERI_DIR)/$$prog\_cheri.c 2>&1 | \
				tee $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_compilation.log; \
		fi; \
	done
	@# Special handling for advanced_protections_cheri_freestanding
	@if [ -f $(CHERI_DIR)/advanced_protections_cheri_freestanding.c ]; then \
		echo "Building advanced_protections_cheri_freestanding for Authentic CHERI..."; \
		$(CHERI_CC) $(CHERI_CFLAGS) -o $(CHERI_DIR)/advanced_protections_cheri_freestanding $(CHERI_DIR)/advanced_protections_cheri_freestanding.c 2>&1 | \
			tee $(RAW_OUTPUTS_DIR)/authentic-cheri/advanced_protections_cheri_freestanding_compilation.log; \
	fi

# Generate assembly output for analysis
generate-assembly:
	@echo "Generating assembly output for analysis..."
	@for prog in $(PROGRAMS); do \
		if [ -f $(RISCV_DIR)/$$prog.c ]; then \
			echo "Generating Standard RISC-V assembly for $$prog..."; \
			$(RISCV_CC) $(RISCV_ASMFLAGS) -o $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog.s $(RISCV_DIR)/$$prog.c; \
		fi; \
		if [ -f $(CHERI_DIR)/$$prog\_cheri.c ]; then \
			echo "Generating Authentic CHERI assembly for $$prog..."; \
			$(CHERI_CC) $(CHERI_ASMFLAGS) -o $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_cheri.s $(CHERI_DIR)/$$prog\_cheri.c; \
		fi; \
	done

# Detailed analysis with debugging information
analyze-detailed:
	@echo "Performing detailed analysis..."
	@for prog in $(PROGRAMS); do \
		if [ -f $(RISCV_DIR)/$$prog.c ]; then \
			echo "Detailed analysis for Standard RISC-V $$prog..."; \
			$(RISCV_CC) $(RISCV_CFLAGS) -fdump-tree-all -fdump-rtl-all \
				-o $(RISCV_DIR)/$$prog\_debug $(RISCV_DIR)/$$prog.c 2>&1 | \
				tee $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_detailed_analysis.log; \
			riscv64-elf-objdump -d -S -l --source $(RISCV_DIR)/$$prog\_debug > \
				$(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_disassembly.log 2>/dev/null || true; \
		fi; \
		if [ -f $(CHERI_DIR)/$$prog\_cheri.c ]; then \
			echo "Detailed analysis for Authentic CHERI $$prog..."; \
			$(CHERI_CC) $(CHERI_CFLAGS) -mllvm -print-before-all -mllvm -print-after-all \
				-o $(CHERI_DIR)/$$prog\_cheri_debug $(CHERI_DIR)/$$prog\_cheri.c 2>&1 | \
				tee $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_detailed_analysis.log; \
			/Users/dlaba556/cheri/output/sdk/bin/llvm-objdump -d -S --mattr=+xcheri \
				$(CHERI_DIR)/$$prog\_cheri_debug > \
				$(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_disassembly.log 2>/dev/null || true; \
		fi; \
	done

# Size comparison analysis
analyze-sizes:
	@echo "Analyzing binary sizes..."
	@echo "Program,Standard RISC-V (bytes),Authentic CHERI (bytes),Overhead (%)" > $(RESULTS_DIR)/size_comparison.csv
	@for prog in $(PROGRAMS); do \
		if [ -f $(RISCV_DIR)/$$prog ] && [ -f $(CHERI_DIR)/$$prog\_cheri ]; then \
			riscv_size=$$(stat -f%z $(RISCV_DIR)/$$prog 2>/dev/null || echo "0"); \
			cheri_size=$$(stat -f%z $(CHERI_DIR)/$$prog\_cheri 2>/dev/null || echo "0"); \
			if [ $$riscv_size -ne 0 ]; then \
				overhead=$$(echo "scale=2; ($$cheri_size - $$riscv_size) * 100 / $$riscv_size" | bc -l 2>/dev/null || echo "0"); \
			else \
				overhead="N/A"; \
			fi; \
			echo "$$prog,$$riscv_size,$$cheri_size,$$overhead" >> $(RESULTS_DIR)/size_comparison.csv; \
		fi; \
	done
	@echo "Size comparison saved to $(RESULTS_DIR)/size_comparison.csv"

# Instruction count analysis
analyze-instructions:
	@echo "Analyzing instruction counts..."
	@echo "Program,Standard RISC-V Instructions,Authentic CHERI Instructions,Difference" > $(RESULTS_DIR)/instruction_comparison.csv
	@for prog in $(PROGRAMS); do \
		if [ -f $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog.s ] && [ -f $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_cheri.s ]; then \
			riscv_count=$$(grep -c "^\s*[a-z]" $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog.s 2>/dev/null || echo "0"); \
			cheri_count=$$(grep -c "^\s*[a-z]" $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_cheri.s 2>/dev/null || echo "0"); \
			diff=$$((cheri_count - riscv_count)); \
			echo "$$prog,$$riscv_count,$$cheri_count,$$diff" >> $(RESULTS_DIR)/instruction_comparison.csv; \
		fi; \
	done
	@echo "Instruction comparison saved to $(RESULTS_DIR)/instruction_comparison.csv"

# Security analysis - identify protection mechanisms
analyze-security:
	@echo "Analyzing security mechanisms..."
	@echo "# Security Analysis Report" > $(RESULTS_DIR)/security_analysis.md
	@echo "Generated on: $$(date)" >> $(RESULTS_DIR)/security_analysis.md
	@echo "" >> $(RESULTS_DIR)/security_analysis.md
	@for prog in $(PROGRAMS); do \
		echo "## $$prog Security Analysis" >> $(RESULTS_DIR)/security_analysis.md; \
		if [ -f $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog.s ]; then \
			echo "### Standard RISC-V" >> $(RESULTS_DIR)/security_analysis.md; \
			echo "\`\`\`" >> $(RESULTS_DIR)/security_analysis.md; \
			grep -E "(ld|sd|lw|sw|addi.*sp)" $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog.s | head -10 >> $(RESULTS_DIR)/security_analysis.md 2>/dev/null || true; \
			echo "\`\`\`" >> $(RESULTS_DIR)/security_analysis.md; \
		fi; \
		if [ -f $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_cheri.s ]; then \
			echo "### Authentic CHERI" >> $(RESULTS_DIR)/security_analysis.md; \
			echo "\`\`\`" >> $(RESULTS_DIR)/security_analysis.md; \
			grep -E "(clc|csc|cincoffset|csetbounds)" $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_cheri.s | head -10 >> $(RESULTS_DIR)/security_analysis.md 2>/dev/null || true; \
			echo "\`\`\`" >> $(RESULTS_DIR)/security_analysis.md; \
		fi; \
		echo "" >> $(RESULTS_DIR)/security_analysis.md; \
	done
	@echo "Security analysis saved to $(RESULTS_DIR)/security_analysis.md"

# Complete analysis pipeline
analyze: compile-all analyze-detailed analyze-sizes analyze-instructions analyze-security
	@echo "Comprehensive analysis complete!"
	@echo "Results available in $(RESULTS_DIR)/"
	@echo "Raw outputs available in $(RAW_OUTPUTS_DIR)/"

# Generate comparison report
compare:
	@echo "Generating comprehensive comparison report..."
	@echo "# CHERI Security Research Platform - Comparison Report" > $(RESULTS_DIR)/comparison_report.md
	@echo "Generated on: $$(date)" >> $(RESULTS_DIR)/comparison_report.md
	@echo "" >> $(RESULTS_DIR)/comparison_report.md
	@echo "## Overview" >> $(RESULTS_DIR)/comparison_report.md
	@echo "This report compares Standard RISC-V and Authentic CHERI implementations across multiple security scenarios." >> $(RESULTS_DIR)/comparison_report.md
	@echo "" >> $(RESULTS_DIR)/comparison_report.md
	@if [ -f $(RESULTS_DIR)/size_comparison.csv ]; then \
		echo "## Size Comparison" >> $(RESULTS_DIR)/comparison_report.md; \
		echo "\`\`\`" >> $(RESULTS_DIR)/comparison_report.md; \
		cat $(RESULTS_DIR)/size_comparison.csv >> $(RESULTS_DIR)/comparison_report.md; \
		echo "\`\`\`" >> $(RESULTS_DIR)/comparison_report.md; \
		echo "" >> $(RESULTS_DIR)/comparison_report.md; \
	fi
	@if [ -f $(RESULTS_DIR)/instruction_comparison.csv ]; then \
		echo "## Instruction Count Comparison" >> $(RESULTS_DIR)/comparison_report.md; \
		echo "\`\`\`" >> $(RESULTS_DIR)/comparison_report.md; \
		cat $(RESULTS_DIR)/instruction_comparison.csv >> $(RESULTS_DIR)/comparison_report.md; \
		echo "\`\`\`" >> $(RESULTS_DIR)/comparison_report.md; \
		echo "" >> $(RESULTS_DIR)/comparison_report.md; \
	fi
	@if [ -f $(RESULTS_DIR)/security_analysis.md ]; then \
		cat $(RESULTS_DIR)/security_analysis.md >> $(RESULTS_DIR)/comparison_report.md; \
	fi
	@echo "Comparison report generated: $(RESULTS_DIR)/comparison_report.md"

# Test programs with sample inputs
test-programs:
	@echo "Testing compiled programs with sample inputs..."
	@for prog in $(PROGRAMS); do \
		if [ -f $(RISCV_DIR)/$$prog ]; then \
			echo "Testing Standard RISC-V $$prog..."; \
			echo "Test: Short input" | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_test.log; \
			$(RISCV_DIR)/$$prog "Safe" 2>&1 | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_test.log || true; \
			echo "Test: Long input" | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_test.log; \
			$(RISCV_DIR)/$$prog "This_is_a_very_long_input_that_will_test_overflow_conditions" 2>&1 | tee -a $(RAW_OUTPUTS_DIR)/standard-riscv/$$prog\_test.log || true; \
		fi; \
		if [ -f $(CHERI_DIR)/$$prog\_cheri ]; then \
			echo "Testing Authentic CHERI $$prog..."; \
			echo "Test: Short input" | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_test.log; \
			$(CHERI_DIR)/$$prog\_cheri "Safe" 2>&1 | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_test.log || true; \
			echo "Test: Long input" | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_test.log; \
			$(CHERI_DIR)/$$prog\_cheri "This_is_a_very_long_input_that_should_trigger_CHERI_protection" 2>&1 | tee -a $(RAW_OUTPUTS_DIR)/authentic-cheri/$$prog\_test.log || true; \
		fi; \
	done

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@find $(RISCV_DIR) -type f -executable -delete 2>/dev/null || true
	@find $(CHERI_DIR) -type f -executable -delete 2>/dev/null || true
	@find . -name "*.o" -delete 2>/dev/null || true
	@find . -name "*.s" -delete 2>/dev/null || true
	@find . -name "*.i" -delete 2>/dev/null || true
	@find . -name "*.ii" -delete 2>/dev/null || true
	@find . -name "*.bc" -delete 2>/dev/null || true
	@find . -name "*_debug" -delete 2>/dev/null || true
	@rm -rf $(RAW_OUTPUTS_DIR)/standard-riscv/* 2>/dev/null || true
	@rm -rf $(RAW_OUTPUTS_DIR)/authentic-cheri/* 2>/dev/null || true
	@rm -rf $(RESULTS_DIR)/* 2>/dev/null || true
	@echo "Clean complete."

# Help target
help:
	@echo "CHERI Security Research Platform - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Complete build and analysis pipeline"
	@echo "  setup            - Create directory structure"
	@echo "  compile-riscv    - Compile Standard RISC-V implementations"
	@echo "  compile-cheri    - Compile Authentic CHERI implementations"
	@echo "  generate-assembly - Generate assembly output for analysis"
	@echo "  analyze          - Perform comprehensive analysis"
	@echo "  analyze-detailed - Detailed compiler analysis with debugging"
	@echo "  analyze-sizes    - Binary size comparison"
	@echo "  analyze-instructions - Instruction count analysis"
	@echo "  analyze-security - Security mechanism analysis"
	@echo "  compare          - Generate comparison report"
	@echo "  test-programs    - Test programs with sample inputs"
	@echo "  clean            - Remove build artifacts"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Fair Comparison Targets:"
	@echo "  fair-comparison  - Run comprehensive fair comparison analysis"
	@echo "  fair-stress-tests - Build CHERI limit-pushing stress tests"
	@echo "  fair-benchmarks  - Run performance comparison benchmarks"
	@echo ""
	@echo "Example usage:"
	@echo "  make all         - Build everything and analyze"
	@echo "  make compile-all - Just compile both architectures"
	@echo "  make compare     - Generate comparison report"
	@echo "  make fair-comparison - Run fair comparison analysis"

# Fair comparison targets (pushing CHERI to its limits)
fair-comparison: fair-stress-tests fair-benchmarks fair-analysis
	@echo "✅ Fair comparison analysis complete"

fair-stress-tests:
	@echo "🧪 Building CHERI stress tests that push limits..."
	@cd extreme-details/edge-cases/stress-tests && \
	$(CHERI_CC) $(CHERI_CFLAGS) -o cheri-limits-stress-test cheri-limits-stress-test.c 2>&1 | tee cheri-limits-build.log && \
	$(CHERI_CC) $(CHERI_CFLAGS) -o performance-comparison performance-comparison.c 2>&1 | tee performance-build.log
	@cd extreme-details/edge-cases/corner-cases && \
	$(CHERI_CC) $(CHERI_CFLAGS) -o advanced-attack-scenarios advanced-attack-scenarios.c 2>&1 | tee advanced-attacks-build.log
	@echo "Stress tests built successfully"

fair-benchmarks:
	@echo "📊 Running fair comparison benchmarks..."
	chmod +x comparative-analysis/fair_comparison_suite.sh
	./comparative-analysis/fair_comparison_suite.sh

fair-analysis:
	@echo "📈 Generating fair comparison analysis..."
	@echo "Fair comparison analysis completed. See results/ directory for detailed reports."
	@echo ""
	@echo "🎯 FAIR COMPARISON SUMMARY:"
	@echo "This analysis includes:"
	@echo "- Performance overhead measurements"
	@echo "- Memory consumption comparisons"
	@echo "- Scenarios where CHERI shows measurable costs"
	@echo "- Advanced attack resistance testing"
	@echo "- Edge cases where protection comes at a price"
