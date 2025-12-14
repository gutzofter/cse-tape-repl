#pragma once
#include <stdint.h>

#define TAPE_MAGIC 0x45504154u /* 'TAPE' */

#define OP_SET             0x01
#define OP_EMIT            0x02
#define OP_WAIT            0x03
#define OP_ASSERT_PRESSURE 0x04
#define OP_END             0x05

#define ASSERT_SCOPE_LAST  0
#define ASSERT_SCOPE_WORST 1

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t header_bytes;
  uint32_t tape_bytes;
  uint32_t code_bytes;
  uint32_t data_bytes;
  uint32_t scenario_id;
  uint32_t flags;
  uint32_t seed;
  uint32_t crc32_all;
  uint32_t crc32_code;
  uint32_t crc32_data;
  uint32_t section_table_off;
  uint32_t reserved[5];
} __attribute__((packed)) tape_header_v1_t;

typedef struct {
  uint8_t  op;
  uint8_t  flags;
  uint16_t len;
} __attribute__((packed)) tape_insn_hdr_t;

typedef struct {
  uint16_t key;
  uint8_t  type;
  uint8_t  reserved;
  uint16_t length;
} __attribute__((packed)) cfg_kv_t;
