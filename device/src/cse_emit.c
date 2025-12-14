#include "cse_wire_v1.h"

void cse_emit_u32(uint16_t event_id, uint32_t value) {
    uint8_t payload[9];
    payload[0] = event_id & 0xFF;
    payload[1] = event_id >> 8;
    payload[2] = 0;
    payload[3] = 4;
    payload[4] = 0;
    payload[5] = value & 0xFF;
    payload[6] = value >> 8;
    payload[7] = value >> 16;
    payload[8] = value >> 24;
    cse_wire_send(CSE_MSG_EVENT, payload, sizeof(payload));
}
