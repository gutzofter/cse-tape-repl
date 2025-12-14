#pragma once
#include <stdint.h>
#include <stdbool.h>

enum {
  CSE_SOF0 = 0xC5,
  CSE_SOF1 = 0x5C,
  CSE_VER1 = 0x01,
};

typedef enum : uint8_t {
  MSG_PING     = 0x01,
  MSG_LOAD     = 0x02,
  MSG_RUN      = 0x03,

  MSG_PONG     = 0x81,
  MSG_LOAD_OK  = 0x82,
  MSG_FINISHED = 0x83,
  MSG_ERR      = 0xFF,
} cse_msg_type_t;

typedef enum : uint8_t {
  RUNMODE_VIRTUAL = 0,
  RUNMODE_REAL    = 1
} cse_run_mode_t;

typedef enum : uint8_t {
  RES_PASS = 0,
  RES_WARN = 1,
  RES_FAIL = 2
} cse_result_t;

typedef enum : uint8_t {
  ERR_OK = 0,
  ERR_BAD_CRC = 1,
  ERR_BAD_LEN = 2,
  ERR_BAD_VER = 3,
  ERR_BAD_TYPE = 4,
  ERR_NO_TAPE = 5,
  ERR_SCENARIO_MISMATCH = 6,
  ERR_TAPE_TOO_BIG = 7,
} cse_err_t;

typedef struct {
  uint8_t  ver;
  uint8_t  type;
  uint8_t  flags;
  uint8_t  seq;
  uint16_t pay_len;  // little-endian on wire
} cse_wire_hdr_t;

typedef struct {
  cse_wire_hdr_t h;
  uint8_t*       payload;
} cse_frame_t;

// ---- Platform UART hooks you implement (Arduino/ESP-IDF wrappers) ----
bool cse_uart_write_all(const uint8_t* buf, uint16_t n);
bool cse_uart_read_byte(uint8_t* out, uint32_t timeout_ms);

// ---- Wire functions ----
uint16_t cse_crc16_ccitt_false(const uint8_t* data, uint16_t len);

bool cse_wire_read_frame(uint8_t* payload_buf, uint16_t payload_cap,
                         cse_frame_t* out, uint32_t timeout_ms);

bool cse_wire_write_frame(uint8_t type, uint8_t seq,
                          const uint8_t* payload, uint16_t pay_len);
