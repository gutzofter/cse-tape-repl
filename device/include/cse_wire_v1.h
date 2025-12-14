#pragma once
#include <stdint.h>
#include <stdbool.h>

#define CSE_WIRE_SOF0 0xC5
#define CSE_WIRE_SOF1 0x5C
#define CSE_WIRE_VER  0x01

bool cse_uart_write(const uint8_t *buf, uint16_t n);
bool cse_uart_read(uint8_t *buf, uint16_t n, uint32_t timeout_ms);
