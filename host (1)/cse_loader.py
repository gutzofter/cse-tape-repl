import argparse, struct, time
import serial

SOF0 = 0xC5
SOF1 = 0x5C
VER1 = 0x01

MSG_PING=0x01
MSG_LOAD=0x02
MSG_RUN =0x03
MSG_PONG=0x81
MSG_LOAD_OK=0x82
MSG_FINISHED=0x83
MSG_ERR=0xFF

PAYLOAD_MAX = 4096
TIMEOUT_S = 2.0

def crc16_ccitt_false(data: bytes) -> int:
    poly = 0x1021
    crc = 0xFFFF
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            crc = ((crc << 1) ^ poly) & 0xFFFF if (crc & 0x8000) else (crc << 1) & 0xFFFF
    return crc

def build_frame(msg_type: int, seq: int, payload: bytes) -> bytes:
    flags = 0
    paylen = len(payload)
    hdr = bytes([SOF0, SOF1, VER1, msg_type, flags, seq]) + struct.pack("<H", paylen)
    core = hdr[2:]  # ver..paylen
    crc = crc16_ccitt_false(core) ^ (crc16_ccitt_false(payload) if payload else 0)
    return hdr + payload + struct.pack("<H", crc)

def read_exact(ser: serial.Serial, n: int) -> bytes:
    out = b""
    t0 = time.time()
    while len(out) < n:
        if time.time() - t0 > TIMEOUT_S:
            raise TimeoutError("read timeout")
        chunk = ser.read(n - len(out))
        if chunk:
            out += chunk
    return out

def read_frame(ser: serial.Serial):
    # scan SOF
    while True:
        b = ser.read(1)
        if not b:
            raise TimeoutError("SOF timeout")
        if b[0] == SOF0:
            b2 = read_exact(ser, 1)
            if b2[0] == SOF1:
                break

    core = read_exact(ser, 6)  # ver,type,flags,seq,paylen(2)
    ver, mtype, flags, seq, paylen = core[0], core[1], core[2], core[3], struct.unpack("<H", core[4:6])[0]
    if ver != VER1:
        raise ValueError(f"bad ver {ver}")
    if paylen > PAYLOAD_MAX:
        raise ValueError(f"bad paylen {paylen}")

    payload = read_exact(ser, paylen) if paylen else b""
    crc_wire = struct.unpack("<H", read_exact(ser, 2))[0]
    crc_calc = crc16_ccitt_false(core) ^ (crc16_ccitt_false(payload) if payload else 0)
    if crc_wire != crc_calc:
        raise ValueError("bad crc")
    return mtype, seq, payload

def cmd_ping(ser):
    ser.write(build_frame(MSG_PING, 1, b""))
    mtype, seq, payload = read_frame(ser)
    if mtype != MSG_PONG:
        raise RuntimeError(f"expected PONG got {hex(mtype)}")
    print("PONG")

def cmd_load(ser, scenario_id: int, tape_path: str):
    tape = open(tape_path, "rb").read()
    payload = struct.pack("<II", scenario_id, len(tape)) + tape
    ser.write(build_frame(MSG_LOAD, 2, payload))
    mtype, seq, payload = read_frame(ser)
    if mtype == MSG_ERR:
        raise RuntimeError(f"ERR code={payload[0] if payload else '??'}")
    if mtype != MSG_LOAD_OK:
        raise RuntimeError(f"expected LOAD_OK got {hex(mtype)}")
    print("LOAD_OK")

def cmd_run(ser, scenario_id: int, runmode: int):
    payload = struct.pack("<IB", scenario_id, runmode)
    ser.write(build_frame(MSG_RUN, 3, payload))
    mtype, seq, payload = read_frame(ser)
    if mtype == MSG_ERR:
        raise RuntimeError(f"ERR code={payload[0] if payload else '??'}")
    if mtype != MSG_FINISHED:
        raise RuntimeError(f"expected FINISHED got {hex(mtype)}")
    sid, res, t_end = struct.unpack("<IBI", payload)
    print(f"FINISHED scenario=0x{sid:08X} result={res} t_end_ms={t_end}")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True)
    ap.add_argument("--baud", type=int, default=115200)
    sub = ap.add_subparsers(dest="cmd", required=True)

    sub.add_parser("ping")
    p_load = sub.add_parser("load")
    p_load.add_argument("scenario_id", type=lambda x: int(x, 0))
    p_load.add_argument("tape")

    p_run = sub.add_parser("run")
    p_run.add_argument("scenario_id", type=lambda x: int(x, 0))
    p_run.add_argument("--mode", choices=["virtual","real"], default="virtual")

    a = ap.parse_args()
    with serial.Serial(a.port, a.baud, timeout=0.2) as ser:
        if a.cmd == "ping":
            cmd_ping(ser)
        elif a.cmd == "load":
            cmd_load(ser, a.scenario_id, a.tape)
        elif a.cmd == "run":
            cmd_run(ser, a.scenario_id, 0 if a.mode=="virtual" else 1)

if __name__ == "__main__":
    main()
