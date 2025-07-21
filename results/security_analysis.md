# Security Analysis Report
Generated on: Mon Jul 21 21:22:03 NZST 2025

## buffer_overflow Security Analysis
### Standard RISC-V
```
	addi	sp,sp,-48	#,,
	sd	ra,40(sp)	#,
	sd	s0,32(sp)	#,
	addi	s0,sp,48	#,,
	sd	a0,-40(s0)	# str, str
	sw	zero,-20(s0)	#, len
	lw	a5,-20(s0)		# tmp141, len
	sw	a5,-20(s0)	# len_9, len
	lw	a5,-20(s0)		# _1, len
	ld	a4,-40(s0)		# tmp143, str
```
### Authentic CHERI
```
	cincoffset	csp, csp, -64
	csc	cra, 48(csp)                    # 16-byte Folded Spill
	csc	cs0, 32(csp)                    # 16-byte Folded Spill
	cincoffset	cs0, csp, 64
	csc	ca0, -48(cs0)
	clc	ca0, -48(cs0)
	cincoffset	ca0, ca0, a1
	clc	cra, 48(csp)                    # 16-byte Folded Reload
	clc	cs0, 32(csp)                    # 16-byte Folded Reload
	cincoffset	csp, csp, 64
```

## use_after_free Security Analysis
### Standard RISC-V
```
	addi	sp,sp,-48	#,,
	sd	ra,40(sp)	#,
	sd	s0,32(sp)	#,
	addi	s0,sp,48	#,,
	sw	a5,-36(s0)	# tmp142, size
	lw	a5,%lo(next_alloc)(a5)		# next_alloc.0_1, next_alloc
	lw	a4,-36(s0)		# tmp146, size
	lw	a4,%lo(next_alloc)(a5)		# next_alloc.1_3, next_alloc
	sd	a5,-24(s0)	# tmp150, ptr
	lw	a5,%lo(next_alloc)(a5)		# next_alloc.2_4, next_alloc
```
### Authentic CHERI
```
	cincoffset	csp, csp, -96
	csc	cra, 80(csp)                    # 16-byte Folded Spill
	csc	cs0, 64(csp)                    # 16-byte Folded Spill
	cincoffset	cs0, csp, 96
	clc	ca0, %pcrel_lo(.LBB0_6)(ca0)
	csc	ca0, -48(cs0)
	clc	ca1, %pcrel_lo(.LBB0_7)(ca1)
	clc	ca0, %pcrel_lo(.LBB0_8)(ca0)
	cincoffset	ca0, ca0, a2
	csc	ca0, -80(cs0)
```

## advanced_vulnerabilities Security Analysis

## advanced_vulnerabilities_freestanding Security Analysis
### Standard RISC-V
```
	addi	sp,sp,-48	#,,
	sd	ra,40(sp)	#,
	sd	s0,32(sp)	#,
	addi	s0,sp,48	#,,
	sd	a0,-40(s0)	# s, s
	sw	zero,-20(s0)	#, len
	lw	a5,-20(s0)		# tmp141, len
	sw	a5,-20(s0)	# len_9, len
	lw	a5,-20(s0)		# _1, len
	ld	a4,-40(s0)		# tmp143, s
```

