#include "cse_types.h"
#include "cse_tape_v0.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

bool bus_emit_event(uint16_t event_type, const uint8_t *payload, uint16_t payload_len);
bool pulse_config_set_u32(uint16_t key, uint32_t v);
bool pulse_config_set_u16(uint16_t key, uint16_t v);
void pulse_doctor_emit_snapshot(void);
bool pulse_get_last_pressure_state(uint16_t metric_id, PressureState *out_state);

typedef struct {
  uint32_t scenario_id;
  uint32_t t_virtual_ms_end;
  uint8_t  result;
  uint8_t  reserved0[3];
  uint32_t primary_metric_id;
  uint32_t primary_model_id;
  int32_t  primary_pressure_q16_16;
  uint8_t  primary_state;
  uint8_t  reserved1[3];
  uint32_t failing_event_id;
  uint32_t trace_available;
} __attribute__((packed)) notify_scenario_finished_t;

void notify_scenario_finished(const notify_scenario_finished_t *fin);

typedef struct { uint32_t now_ms; } cse_clock_t;
static inline void clock_reset(cse_clock_t *c) { c->now_ms = 0; }
static inline void clock_advance_ms(cse_clock_t *c, uint32_t d) { c->now_ms += d; }

typedef struct {
  uint32_t scenario_id;
  cse_clock_t vclock;
  ScenarioResult result;
  uint16_t primary_metric_id;
  PressureState primary_state;
} tape_runtime_t;

static bool tape_parse(const uint8_t *tape, uint32_t n,
                       tape_header_v1_t *hdr,
                       const uint8_t **code, uint32_t *code_n) {
  if (n < sizeof(tape_header_v1_t)) return false;
  memcpy(hdr, tape, sizeof(*hdr));
  if (hdr->magic != TAPE_MAGIC) return false;
  if (hdr->version != 0x0001) return false;
  if (hdr->header_bytes < sizeof(tape_header_v1_t)) return false;
  if (hdr->tape_bytes != n) return false;

  uint32_t code_off = hdr->header_bytes;
  if (code_off + hdr->code_bytes > n) return false;
  *code = tape + code_off; *code_n = hdr->code_bytes;
  return true;
}

void cse_tape_run_v0(const uint8_t *tape, uint32_t tape_len) {
  tape_header_v1_t hdr;
  const uint8_t *code = 0;
  uint32_t code_n = 0;
  if (!tape_parse(tape, tape_len, &hdr, &code, &code_n)) return;

  tape_runtime_t rt = {0};
  rt.scenario_id = hdr.scenario_id;
  rt.result = RESULT_PASS;
  rt.primary_state = PRESSURE_OK;
  clock_reset(&rt.vclock);

  uint32_t pc = 0;
  while (pc < code_n) {
    if (pc + sizeof(tape_insn_hdr_t) > code_n) break;
    const tape_insn_hdr_t *ih = (const tape_insn_hdr_t*)(code + pc);
    pc += sizeof(*ih);
    if (pc + ih->len > code_n) break;
    const uint8_t *body = code + pc;
    pc += ih->len;

    (void)body;
    /* TODO: implement OP_SET/OP_EMIT/OP_WAIT/OP_ASSERT/OP_END from full skeleton */
  }

  notify_scenario_finished_t fin = {0};
  fin.scenario_id = rt.scenario_id;
  fin.t_virtual_ms_end = rt.vclock.now_ms;
  fin.result = (uint8_t)rt.result;
  fin.primary_metric_id = (uint32_t)rt.primary_metric_id;
  fin.primary_state = (uint8_t)rt.primary_state;
  notify_scenario_finished(&fin);
}

void cse_tape_run(const uint8_t *tape, uint32_t tape_len) {
  cse_tape_run_v0(tape, tape_len);
}
