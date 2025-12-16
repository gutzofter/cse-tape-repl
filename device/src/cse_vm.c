#include "cse_vm.h"
#include "cse_opcodes.h"
#include "cse_wire_v1.h"
#include "cse_emit.h"
#include <string.h>

static uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

void cse_vm_init(cse_vm_t *vm, const uint8_t *tape, uint32_t tape_len) {
    memset(vm, 0, sizeof(*vm));
    vm->tape = tape;
    vm->tape_len = tape_len;
    vm->sp = -1;
    vm->running = true;
}

void cse_vm_run(cse_vm_t *vm) {
    while (vm->running && vm->pc < vm->tape_len) {
        uint8_t op = vm->tape[vm->pc++];
        vm->steps_executed++;

        switch (op) {
        case OP_PUSH_U32: {
            if (vm->pc + 4 > vm->tape_len || vm->sp + 1 >= CSE_STACK_MAX) goto fault;
            uint32_t v = read_u32_le(&vm->tape[vm->pc]);
            vm->pc += 4;
            vm->stack[++vm->sp] = v;
            break;
        }
        case OP_ADD_U32: {
            if (vm->sp < 1) goto fault;
            uint32_t b = vm->stack[vm->sp--];
            vm->stack[vm->sp] += b;
            break;
        }
        case OP_EMIT_U32: {
            if (vm->pc + 2 > vm->tape_len || vm->sp < 0) goto fault;
            uint16_t eid = vm->tape[vm->pc] | (vm->tape[vm->pc+1] << 8);
            vm->pc += 2;
            cse_emit_u32(eid, vm->stack[vm->sp--]);
            break;
        }
        case OP_HALT:
            vm->running = false;
            vm->halt_result = vm->tape[vm->pc++];
            break;
        case OP_NOP:
            break;
        default:
            goto fault;
        }
    }
    return;
fault:
    vm->running = false;
    vm->halt_result = 2;
}
