#pragma once
#include <stdint.h>
#include <stdbool.h>

#define CSE_STACK_MAX 32

typedef struct {
    const uint8_t *tape;
    uint32_t tape_len;
    uint32_t pc;

    uint32_t stack[CSE_STACK_MAX];
    int32_t  sp;

    bool     running;
    uint8_t  halt_result;

    uint32_t steps_executed;
} cse_vm_t;

void cse_vm_init(cse_vm_t *vm, const uint8_t *tape, uint32_t tape_len);
void cse_vm_run(cse_vm_t *vm);
