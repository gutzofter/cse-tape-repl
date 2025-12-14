# CSE VM Architecture

The CSE VM is a deterministic stack machine designed for constrained devices.

## Components
- Tape (immutable bytecode)
- Stack (fixed size)
- Program Counter
- Execution Loop

## Invariants
- No heap allocation
- No RTOS dependency
- Bounded memory
- Explicit faults

## Observability
Execution emits events via the CSE Wire protocol.
