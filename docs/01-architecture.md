# Architecture (Presenter-first / EventMapper + Tape Runtime)

Scenario Source → Compiler → Tape → UART Load → On-device Interpreter → Doctor + Verdict → Host Monitor

Key invariants:
- No direct coupling between models: interaction is by message/events.
- Time is explicit (virtual clock for replay/simulation).
- Pressure is computed by model settings (no magic constants in code).
