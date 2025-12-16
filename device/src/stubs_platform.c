#include "cse_types.h"
#include "cse_wire_v1.h"
#include <stdbool.h>
#include <stdint.h>

bool bus_emit_event(uint16_t event_type, const uint8_t *payload, uint16_t payload_len) {
  (void)event_type; (void)payload; (void)payload_len;
  return true;
}

bool cse_uart_write_all(const uint8_t* buf, uint16_t n) {
  // TODO: wire this to your board's UART write API
  (void)buf; (void)n;
  return true;
}

bool cse_uart_read_byte(uint8_t* out, uint32_t timeout_ms) {
  // TODO: wire this to your board's UART read API
  (void)timeout_ms;
  if (out) *out = 0;
  return false;
}
bool pulse_config_set_u32(uint16_t key, uint32_t v) { (void)key; (void)v; return true; }
bool pulse_config_set_u16(uint16_t key, uint16_t v) { (void)key; (void)v; return true; }
void pulse_doctor_emit_snapshot(void) { }
bool pulse_get_last_pressure_state(uint16_t metric_id, PressureState *out_state) {
  (void)metric_id; if (out_state) *out_state = PRESSURE_OK; return true;
}

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

void notify_scenario_finished(const notify_scenario_finished_t *fin) { (void)fin; }
