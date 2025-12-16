#include "cse_wire_v1.h"
#include "cse_emit.h"

void cse_emit_u32(uint16_t event_id, uint32_t value) {
    // Payload: [u16 event_id][u32 value]
    uint8_t payload[6];
    payload[0] = (uint8_t)(event_id & 0xFF);
    payload[1] = (uint8_t)((event_id >> 8) & 0xFF);
    payload[2] = (uint8_t)(value & 0xFF);
    payload[3] = (uint8_t)((value >> 8) & 0xFF);
    payload[4] = (uint8_t)((value >> 16) & 0xFF);
    payload[5] = (uint8_t)((value >> 24) & 0xFF);
    // Sequence number is best-effort here; 0 keeps it deterministic without state.
    (void)cse_wire_write_frame((uint8_t)MSG_EVENT, 0, payload, (uint16_t)sizeof(payload));
}
