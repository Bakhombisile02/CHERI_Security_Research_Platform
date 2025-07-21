# Detailed Assembly Analysis Report

## Overview
This report provides instruction-by-instruction analysis of Standard RISC-V vs Authentic CHERI implementations.

Generated on: Mon Jul 21 21:09:27 NZST 2025

## buffer_overflow Analysis

### Standard RISC-V Implementation
```assembly
main:
	addi	sp,sp,-16	#,,
	sd	ra,8(sp)	#,
	sd	s0,0(sp)	#,
	addi	s0,sp,16	#,,
# implementations/standard-riscv/buffer_overflow.c:72:     test_buffer_overflow();
	call	test_buffer_overflow		#
# implementations/standard-riscv/buffer_overflow.c:73:     stack_canary_function();
	call	stack_canary_function		#
# implementations/standard-riscv/buffer_overflow.c:74:     return 0;
	li	a5,0		# _4,
# implementations/standard-riscv/buffer_overflow.c:75: }
	mv	a0,a5	#, <retval>
	ld	ra,8(sp)		#,
	ld	s0,0(sp)		#,
	addi	sp,sp,16	#,,
	jr	ra		#
	.size	main, .-main
	.section	.rodata
	.align	3
.LC0:
	.dword	-2401053089206453570
	.ident	"GCC: (GNU) 15.1.0"
	.section	.note.GNU-stack,"",@progbits
```

### Authentic CHERI Implementation
```assembly
main:                                   # @main
# %bb.0:                                # %entry
	cincoffset	csp, csp, -32
	csc	cra, 16(csp)                    # 16-byte Folded Spill
	csc	cs0, 0(csp)                     # 16-byte Folded Spill
	cincoffset	cs0, csp, 32
	ccall	test_cheri_protection
	li	a0, 0
	clc	cra, 16(csp)                    # 16-byte Folded Reload
	clc	cs0, 0(csp)                     # 16-byte Folded Reload
	cincoffset	csp, csp, 32
	cret
.Lfunc_end8:
	.size	main, .Lfunc_end8-main
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"Safe"
	.size	.L.str, 5

	.type	.L.str.1,@object                # @.str.1
.L.str.1:
	.asciz	"1234567"
	.size	.L.str.1, 8

	.type	.L.str.2,@object                # @.str.2
.L.str.2:
	.asciz	"TooLong"
	.size	.L.str.2, 8

	.ident	"clang version 15.0.0 (https://github.com/CTSRD-CHERI/llvm-project.git 578ea4f7ef67d589f0ca7d10ec9e383333567421)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym simple_strlen
	.addrsig_sym cheri_strcpy
	.addrsig_sym cheri_length_get
	.addrsig_sym protected_function
	.addrsig_sym cheri_bounds_set
	.addrsig_sym cheri_tag_get
	.addrsig_sym demonstrate_bounds_checking
	.addrsig_sym test_cheri_protection
```

### Key Differences

- Standard RISC-V loads: 33
- Standard RISC-V stores: 28
- CHERI capability loads: 35
- CHERI capability stores: 31
- CHERI bounds operations: 34

## use_after_free Analysis

### Standard RISC-V Implementation
```assembly
main:
	addi	sp,sp,-16	#,,
	sd	ra,8(sp)	#,
	sd	s0,0(sp)	#,
	addi	s0,sp,16	#,,
# implementations/standard-riscv/use_after_free.c:77:     test_use_after_free();
	call	test_use_after_free		#
# implementations/standard-riscv/use_after_free.c:78:     return 0;
	li	a5,0		# _3,
# implementations/standard-riscv/use_after_free.c:79: }
	mv	a0,a5	#, <retval>
	ld	ra,8(sp)		#,
	ld	s0,0(sp)		#,
	addi	sp,sp,16	#,,
	jr	ra		#
	.size	main, .-main
	.ident	"GCC: (GNU) 15.1.0"
	.section	.note.GNU-stack,"",@progbits
```

### Authentic CHERI Implementation
```assembly
main:                                   # @main
# %bb.0:                                # %entry
	cincoffset	csp, csp, -32
	csc	cra, 16(csp)                    # 16-byte Folded Spill
	csc	cs0, 0(csp)                     # 16-byte Folded Spill
	cincoffset	cs0, csp, 32
	ccall	test_cheri_protection
	li	a0, 0
	clc	cra, 16(csp)                    # 16-byte Folded Reload
	clc	cs0, 0(csp)                     # 16-byte Folded Reload
	cincoffset	csp, csp, 32
	cret
.Lfunc_end9:
	.size	main, .Lfunc_end9-main
                                        # -- End function
	.type	next_alloc,@object              # @next_alloc
	.section	.sbss,"aw",@nobits
	.p2align	2
next_alloc:
	.word	0                               # 0x0
	.size	next_alloc, 4

	.type	memory_pool,@object             # @memory_pool
	.local	memory_pool
	.comm	memory_pool,1024,1
	.type	num_caps,@object                # @num_caps
	.p2align	2
num_caps:
	.word	0                               # 0x0
	.size	num_caps, 4

	.type	allocated_caps,@object          # @allocated_caps
	.local	allocated_caps
	.comm	allocated_caps,512,16
	.ident	"clang version 15.0.0 (https://github.com/CTSRD-CHERI/llvm-project.git 578ea4f7ef67d589f0ca7d10ec9e383333567421)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym cheri_malloc
	.addrsig_sym cheri_bounds_set
	.addrsig_sym cheri_free
	.addrsig_sym cheri_tag_clear
	.addrsig_sym protected_use_after_free
	.addrsig_sym cheri_tag_get
	.addrsig_sym protected_double_free
	.addrsig_sym demonstrate_bounds_protection
	.addrsig_sym test_cheri_protection
	.addrsig_sym next_alloc
	.addrsig_sym memory_pool
	.addrsig_sym num_caps
	.addrsig_sym allocated_caps
```

### Key Differences

- Standard RISC-V loads: 32
- Standard RISC-V stores: 25
- CHERI capability loads: 54
- CHERI capability stores: 36
- CHERI bounds operations: 35

