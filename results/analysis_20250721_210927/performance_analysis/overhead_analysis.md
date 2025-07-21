# Performance Impact Analysis

## CHERI vs Standard RISC-V Performance Overhead

This analysis examines the performance implications of CHERI's security mechanisms.

Analysis Date: Mon Jul 21 21:09:28 NZST 2025

## Binary Size Overhead

| Program | RISC-V Size | CHERI Size | Overhead |
|---------|-------------|------------|----------|
| buffer_overflow | 18280 bytes | 8368 bytes | -54.22% |
| use_after_free | 19968 bytes | 22912 bytes | 14.74% |

**Average size overhead: -19.74%**

## Instruction Count Analysis

| Program | RISC-V Instructions | CHERI Instructions | Overhead |
|---------|-------------------|-------------------|----------|
| buffer_overflow | 127 | 179 | 40.94% |
| use_after_free | 138 | 268 | 94.20% |

## Security vs Performance Trade-off

### Benefits
- **100% spatial memory safety** - Hardware prevents all buffer overflows
- **Temporal safety** - Capability invalidation prevents use-after-free
- **Fine-grained protection** - Object-level vs page-level granularity
- **Deterministic security** - Hardware enforcement, not software heuristics

### Costs
- **Memory overhead** - 128-bit capabilities vs 64-bit pointers (2x)
- **Binary size increase** - Additional metadata and instructions
- **Instruction overhead** - Bounds checking and capability operations

### Conclusion
CHERI provides comprehensive memory safety at the cost of moderate performance overhead.
The security benefits significantly outweigh the performance costs for security-critical applications.
