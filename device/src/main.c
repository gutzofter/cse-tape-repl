#include "cse_wire_v1.h"
#include "cse_types.h"
#include "cse_tape_v0.h"
#include <stdint.h>
#include <stdbool.h>

enum {
  MAX_TAPE_BYTES = 2048,
  PAYLOAD_CAP    = 2200,
};

static uint8_t g_payload[PAYLOAD_CAP];
static uint8_t g_tape[MAX_TAPE_BYTES];
static uint32_t g_tape_len = 0;
static uint32_t g_loaded_scenario = 0;
static bool     g_has_tape = false;

static uint32_t rd_u32_le(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void reply_err(uint8_t seq, cse_err_t code) {
  uint8_t p[1] = { (uint8_t)code };
  (void)cse_wire_write_frame(MSG_ERR, seq, p, (uint16_t)sizeof(p));
}

// Call this from Arduino loop() or an ESP-IDF task
void cse_wire_loop_once(void) {
  cse_frame_t fr;
  if (!cse_wire_read_frame(g_payload, (uint16_t)sizeof(g_payload), &fr, 2000)) {
    return;
  }

  switch ((cse_msg_type_t)fr.h.type) {
    case MSG_PING: {
      (void)cse_wire_write_frame(MSG_PONG, fr.h.seq, 0, 0);
      break;
    }

    case MSG_LOAD: {
      if (fr.h.pay_len < 8) { reply_err(fr.h.seq, ERR_BAD_LEN); break; }
      const uint32_t scenario_id = rd_u32_le(&fr.payload[0]);
      const uint32_t tape_len    = rd_u32_le(&fr.payload[4]);
      const uint32_t avail       = (uint32_t)fr.h.pay_len - 8u;

      if (tape_len != avail) { reply_err(fr.h.seq, ERR_BAD_LEN); break; }
      if (tape_len > (uint32_t)MAX_TAPE_BYTES) { reply_err(fr.h.seq, ERR_TAPE_TOO_BIG); break; }

      for (uint32_t i = 0; i < tape_len; i++) g_tape[i] = fr.payload[8 + i];
      g_tape_len = tape_len;
      g_loaded_scenario = scenario_id;
      g_has_tape = true;

      uint8_t ok[1] = { (uint8_t)ERR_OK };
      (void)cse_wire_write_frame(MSG_LOAD_OK, fr.h.seq, ok, (uint16_t)sizeof(ok));
      break;
    }

    case MSG_RUN: {
      if (fr.h.pay_len < 5) { reply_err(fr.h.seq, ERR_BAD_LEN); break; }
      if (!g_has_tape) { reply_err(fr.h.seq, ERR_NO_TAPE); break; }

      const uint32_t scenario_id = rd_u32_le(&fr.payload[0]);
      if (scenario_id != g_loaded_scenario) { reply_err(fr.h.seq, ERR_SCENARIO_MISMATCH); break; }

      // run mode reserved for later
      (void)fr.payload[4];

      cse_tape_run(g_tape, g_tape_len);

      // FINISHED payload: [u32 scenario_id][u8 result][u32 t_end_ms]
      uint8_t fin[9];
      fin[0]= (uint8_t)(scenario_id & 0xFF);
      fin[1]= (uint8_t)((scenario_id >> 8) & 0xFF);
      fin[2]= (uint8_t)((scenario_id >> 16) & 0xFF);
      fin[3]= (uint8_t)((scenario_id >> 24) & 0xFF);
      fin[4]= (uint8_t)RES_FAIL; // replace with real interpreter result later
      const uint32_t t_end_ms = 0;
      fin[5]= (uint8_t)(t_end_ms & 0xFF);
      fin[6]= (uint8_t)((t_end_ms >> 8) & 0xFF);
      fin[7]= (uint8_t)((t_end_ms >> 16) & 0xFF);
      fin[8]= (uint8_t)((t_end_ms >> 24) & 0xFF);

      (void)cse_wire_write_frame(MSG_FINISHED, fr.h.seq, fin, (uint16_t)sizeof(fin));
      break;
    }

    default:
      reply_err(fr.h.seq, ERR_BAD_TYPE);
      break;
  }
}

void setup(void) {
  // TODO: initialize UART pins and baud for cse_wire_v1
}

void loop(void) {
  cse_wire_loop_once();
}
