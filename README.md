# CSE Tape + REPL (v0)

Minimal, end-to-end vertical slice for a deterministic **Scenario Tape** runtime on ESP32-class hardware.

Core loop:

- Scenario source (tiny line-based format) → compiles into a Tape (binary)
- Tape loads over UART (CSE Wire v1) → runs on-device (virtual clock default)
- Device emits Doctor snapshots + a single Scenario Finished verdict
- Same tape runs on hardware and in simulation

This repo is intentionally scoped to the Theory of Constraints bottleneck:
> **Scenario → Insight latency**

---

## Repository Layout

```
cse-tape-repl/
  README.md

  docs/
    01-architecture.md
    02-wire-protocol-cse-wire-v1.md
    03-tape-format-v0.md
    04-scenario-compiler-input-v0.md
    05-demo-holy-shit.md

  device/
    include/
      cse_wire_v1.h
      cse_tape_v0.h
      cse_types.h
    src/
      cse_tape_interpreter.c
      cse_wire_v1.c
      stubs_platform.c
    platformio.ini
    README-device.md

  host/
    cse_uart_monitor.py
    cse_scenario_compiler.py
    cse_loader.py
    requirements.txt
    README-host.md

  scenarios/
    comms_window_minimal.scn
    latency_smoke.scn
    queue_overflow.scn
```

---

## Quick Start (Host)

```bash
cd host
python -m venv .venv
source .venv/bin/activate  # (Windows: .venv\Scripts\activate)
pip install -r requirements.txt
python cse_scenario_compiler.py ../scenarios/comms_window_minimal.scn -o comms_window_minimal.tape
```

Loader/monitor will work once `host/*` framing is implemented:

```bash
python cse_loader.py --port /dev/ttyUSB0 load comms_window_minimal.tape
python cse_loader.py --port /dev/ttyUSB0 run  0x00001234 --mode VIRTUAL
python cse_uart_monitor.py /dev/ttyUSB0 115200
```

---

## Quick Start (Device)

`device/src/cse_tape_interpreter.c` is a compilable skeleton once you implement the platform hooks:

- `bus_emit_event(event_type, payload, len)`
- `pulse_config_set_*`
- `pulse_doctor_emit_snapshot()` + `pulse_get_last_pressure_state(metric_id, &out)`
- `notify_scenario_finished(&fin)` (serialize + send over CSE Wire v1)

See `device/src/stubs_platform.c` for placeholders.

---

## What’s Implemented vs Stubbed

Implemented:
- Specs: wire protocol, tape format, compiler input format
- Device: tape interpreter skeleton + stubs

Stubbed:
- UART framing encode/decode (device + host)
- CRC16/CRC32 routines (or use existing libs)
- Real Pulse/Doctor integration
- DATAREF support (later)

License: pick later.
