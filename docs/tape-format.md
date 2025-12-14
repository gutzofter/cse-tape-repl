# CSE Tape Format v0.1

This document defines the executable tape format for the CSE VM.

## Design Goals
- Deterministic execution
- Byte-oriented
- Replayable
- Minimal parsing overhead
- ESP32-friendly

## Header Layout (little-endian)

| Field | Size |
|------|------|
| Magic ("CSET") | 4 |
| Version | 1 |
| Flags | 1 |
| HeaderLen | 2 |
| ScenarioId | 4 |
| TapeLen | 4 |
| EntryPoint | 2 |
| Reserved | 2 |
| CRC32 | 4 |

Minimum header length: 24 bytes.

## Bytecode Instructions

| Opcode | Mnemonic |
|------|----------|
| 0x01 | PUSH_U32 |
| 0x02 | ADD_U32 |
| 0x10 | EMIT_U32 |
| 0x20 | HALT |
| 0x21 | NOP |

Operands are little-endian.

## Execution Model
- Single-threaded
- Stack-based
- No implicit timing
- Explicit HALT
