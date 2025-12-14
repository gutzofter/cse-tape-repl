import argparse, re, struct
from pathlib import Path

TAPE_MAGIC = 0x45504154  # 'TAPE'
VERSION = 0x0001

OP_SET  = 0x01
OP_EMIT = 0x02
OP_WAIT = 0x03
OP_ASSERT_PRESSURE = 0x04
OP_END  = 0x05

VAL_U32=1
VAL_U16=2

KEYS = {"EVENT_TIME_BUDGET_US": (1, VAL_U32), "LANE_CAP_DEFAULT": (3, VAL_U16)}
METRICS = {"LAT":1, "INS":2, "QUE":3}
STATES  = {"OK":0, "WARN":1, "FAIL":2}
RESULTS = {"PASS":0, "WARN":1, "FAIL":2}

def parse_int(tok: str) -> int:
    tok = tok.strip()
    if tok.startswith(("0x","0X")): return int(tok, 16)
    return int(tok, 10)

def insn(op, flags, body: bytes) -> bytes:
    return struct.pack("<BBH", op, flags, len(body)) + body

def compile_scn(text: str) -> bytes:
    scenario_id = None
    code = bytearray()

    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("#"): continue
        toks = line.split()
        head = toks[0]

        if head == "SCENARIO":
            scenario_id = parse_int(toks[1]); continue

        if head == "SET":
            k, v = " ".join(toks[1:]).split("=", 1)
            key_id, vtype = KEYS[k.strip()]
            if vtype == VAL_U32: vb = struct.pack("<I", parse_int(v))
            else: vb = struct.pack("<H", parse_int(v))
            body = struct.pack("<HBBH", key_id, vtype, 0, len(vb)) + vb
            code += insn(OP_SET, 0, body); continue

        if head == "EMIT":
            event_type = parse_int(toks[1])
            mode = toks[2]
            if mode == "HEX":
                hx = re.sub(r"\s+", "", " ".join(toks[3:]))
                payload = bytes.fromhex(hx)
            else:
                s = " ".join(toks[3:]).strip()
                payload = s[1:-1].encode("utf-8")
            body = struct.pack("<HH", event_type, len(payload)) + payload
            code += insn(OP_EMIT, 0, body); continue

        if head == "WAIT":
            code += insn(OP_WAIT, 0, struct.pack("<I", parse_int(toks[1]))); continue

        if head == "ASSERT_PRESSURE":
            metric = METRICS[toks[1]]
            expected = STATES[toks[2]]
            scope = 0
            timeout = 0
            body = struct.pack("<HBBI", metric, expected, scope, timeout)
            code += insn(OP_ASSERT_PRESSURE, 0, body); continue

        if head == "END":
            res = RESULTS[toks[1]]
            code += insn(OP_END, 0, struct.pack("<B3s", res, b"\x00\x00\x00")); continue

        raise ValueError("Unknown: " + head)

    if scenario_id is None: raise ValueError("Missing SCENARIO")
    header_size = 72
    tape_total = header_size + len(code)
    hdr = struct.pack(
        "<IHHIIIIIIIII I I I I 5I".replace(" ", ""),
        TAPE_MAGIC, VERSION, header_size,
        tape_total, len(code), 0,
        scenario_id, 0, 0,
        0,0,0,
        0,
        0,0,0,0,0
    )
    return hdr + bytes(code)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("scenario_path")
    ap.add_argument("-o", "--out", required=True)
    a = ap.parse_args()
    tape = compile_scn(Path(a.scenario_path).read_text(encoding="utf-8"))
    Path(a.out).write_bytes(tape)
    print("Wrote", a.out, len(tape), "bytes")

if __name__ == "__main__":
    main()
