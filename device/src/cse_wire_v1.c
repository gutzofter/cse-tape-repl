#include "cse_wire_v1.h"

enum {
  WIRE_HDR_BYTES = 4 /*SOF+SOF*/ + 1 /*ver*/ + 1 /*type*/ + 1 /*flags*/ + 1 /*seq*/ + 2 /*paylen*/,
  WIRE_CORE_HDR_BYTES = 1+1+1+1+2, // ver..paylen
  WIRE_CRC_BYTES = 2,
};

static inline uint16_t le16(const uint8_t lo, const uint8_t hi) {
  return (uint16_t)lo | ((uint16_t)hi << 8);
}
static inline void wr_le16(uint8_t* p, uint16_t v) {
  p[0] = (uint8_t)(v & 0xFF);
  p[1] = (uint8_t)((v >> 8) & 0xFF);
}

uint16_t cse_crc16_ccitt_false(const uint8_t* data, uint16_t len) {
  // CRC-16/CCITT-FALSE: poly=0x1021 init=0xFFFF xorout=0x0000 refin=false refout=false
  const uint16_t POLY = 0x1021;
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t b = 0; b < 8; b++) {
      crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ POLY) : (uint16_t)(crc << 1);
    }
  }
  return crc;
}

static bool read_exact(uint8_t* buf, uint16_t n, uint32_t timeout_ms) {
  for (uint16_t i = 0; i < n; i++) {
    if (!cse_uart_read_byte(&buf[i], timeout_ms)) return false;
  }
  return true;
}

bool cse_wire_read_frame(uint8_t* payload_buf, uint16_t payload_cap,
                         cse_frame_t* out, uint32_t timeout_ms) {
  if (!out || !payload_buf) return false;

  // Scan for SOF
  uint8_t b = 0;
  while (true) {
    if (!cse_uart_read_byte(&b, timeout_ms)) return false;
    if (b == (uint8_t)CSE_SOF0) {
      uint8_t b2 = 0;
      if (!cse_uart_read_byte(&b2, timeout_ms)) return false;
      if (b2 == (uint8_t)CSE_SOF1) break;
    }
  }

  // Read core header (ver..paylen)
  uint8_t core[WIRE_CORE_HDR_BYTES];
  if (!read_exact(core, (uint16_t)sizeof(core), timeout_ms)) return false;

  const uint8_t ver   = core[0];
  const uint8_t type  = core[1];
  const uint8_t flags = core[2];
  const uint8_t seq   = core[3];
  const uint16_t pay_len = le16(core[4], core[5]);

  if (ver != (uint8_t)CSE_VER1) return false;
  if (pay_len > payload_cap) return false;

  if (!read_exact(payload_buf, pay_len, timeout_ms)) return false;

  uint8_t crc_le[WIRE_CRC_BYTES];
  if (!read_exact(crc_le, (uint16_t)sizeof(crc_le), timeout_ms)) return false;
  const uint16_t crc_wire = le16(crc_le[0], crc_le[1]);

  // CRC over core header + payload (v1-lite: two-pass mix)
  uint16_t crc_calc = cse_crc16_ccitt_false(core, (uint16_t)sizeof(core));
  if (pay_len) {
    crc_calc = cse_crc16_ccitt_false(payload_buf, pay_len) ^ crc_calc;
  }

  if (crc_calc != crc_wire) return false;

  out->h.ver = ver;
  out->h.type = type;
  out->h.flags = flags;
  out->h.seq = seq;
  out->h.pay_len = pay_len;
  out->payload = payload_buf;
  return true;
}

bool cse_wire_write_frame(uint8_t type, uint8_t seq,
                          const uint8_t* payload, uint16_t pay_len) {
  // Build header
  uint8_t hdr[WIRE_HDR_BYTES];
  hdr[0] = (uint8_t)CSE_SOF0;
  hdr[1] = (uint8_t)CSE_SOF1;
  hdr[2] = (uint8_t)CSE_VER1;
  hdr[3] = type;
  hdr[4] = 0;     // FLAGS (unused in v1-lite)
  hdr[5] = seq;
  wr_le16(&hdr[6], pay_len);

  // CRC over core header (ver..paylen) + payload (v1-lite: two-pass mix)
  const uint8_t* core = &hdr[2];
  const uint16_t core_len = (uint16_t)(WIRE_CORE_HDR_BYTES);
  uint16_t crc = cse_crc16_ccitt_false(core, core_len);
  if (pay_len && payload) {
    crc = cse_crc16_ccitt_false(payload, pay_len) ^ crc;
  }

  uint8_t crc_out[WIRE_CRC_BYTES];
  wr_le16(crc_out, crc);

  if (!cse_uart_write_all(hdr, (uint16_t)sizeof(hdr))) return false;
  if (pay_len && payload) {
    if (!cse_uart_write_all(payload, pay_len)) return false;
  }
  if (!cse_uart_write_all(crc_out, (uint16_t)sizeof(crc_out))) return false;
  return true;
}
