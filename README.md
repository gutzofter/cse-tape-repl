# CSE Wire v1 (ESP32 / PlatformIO)

This repository contains a **deterministic, brokerless host ⇄ device protocol** and minimal runtime for ESP32-class microcontrollers.

The goal is to provide a **replayable, simulatable, message-driven execution loop** suitable for constrained systems, testing, and uplink-style workflows.

This is **not** a messaging framework and **not** RTOS-based.

---

## What This Is

- A framed UART wire protocol (**CSE Wire v1-lite**)
- A minimal device dispatcher (PING / LOAD / RUN)
- A host-side loader tool
- A foundation for tape-based execution and diagnostics

Everything is designed to be:
- Deterministic
- Explicitly framed (no stream parsing)
- Replayable
- Easy to bridge to higher-level systems later (e.g. ZMQ)

---

## What This Is Not

- Not RTOS-dependent
- Not async / threaded
- Not a general-purpose message bus
- Not optimized yet

Those come **after** the core constraint is proven.

---

## Repository Structure

```
.
├── platformio.ini
├── device/
│   ├── include/
│   │   └── cse_wire_v1.h
│   └── src/
│       ├── cse_wire_v1.c
│       └── main.c
├── host/
│   └── cse_loader.py
└── README.md
```

---

## Build & Flash (PlatformIO)

1. Install VS Code
2. Install the PlatformIO extension
3. Open this repository as a PlatformIO project
4. Select your ESP32 environment
5. Build and flash normally via PlatformIO

> **Note:** Platform-specific UART hooks must be implemented:
> - `cse_uart_write_all`
> - `cse_uart_read_byte`
>
> These are intentionally left as thin wrappers so the protocol remains platform-agnostic.

---

## Host Tool (Python)

The host loader communicates with the device over UART.

### Requirements

```
pip install pyserial
```

### Usage

```bash
python host/cse_loader.py --port /dev/ttyUSB0 ping
python host/cse_loader.py --port /dev/ttyUSB0 load 0x00001234 scenarios/example.tape
python host/cse_loader.py --port /dev/ttyUSB0 run  0x00001234 --mode virtual
```

Expected output:

```
PONG
LOAD_OK
FINISHED scenario=0x00001234 result=FAIL t_end_ms=0
```

---

## Protocol Overview (v1-lite)

Frame format:

```
[SOF0][SOF1][VER][TYPE][FLAGS][SEQ][PAYLEN][PAYLOAD...][CRC16]
```

- Explicit framing
- Length-prefixed payloads
- CRC-16/CCITT-FALSE
- No implicit state

Current message set:
- `PING / PONG`
- `LOAD / LOAD_OK`
- `RUN / FINISHED`
- `ERR`

---

## Development Notes

- Do **not** refactor the wire protocol without versioning
- Do **not** introduce RTOS dependencies
- Do **not** add background tasks or threading

This project intentionally proves the **minimum viable execution loop** first.

---

## Next Planned Steps

- Chunked LOAD with ACK/NACK
- Virtual clock facade
- Diagnostic “Doctor” message stream
- Tape compiler / DSL
- ZMQ gateway (host-side)

---

## License

This project is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).

Commercial licensing options may be available.
