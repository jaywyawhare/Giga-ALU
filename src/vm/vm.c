#include "vm/vm.h"

#include <string.h>

GigaInstruction giga_decode_instruction(uint16_t raw_word) {
    GigaInstruction instruction;
    instruction.raw = raw_word;
    instruction.opcode = (GigaOpcode)((raw_word >> 12) & 0x0F);
    instruction.dest_reg = (uint8_t)((raw_word >> 8) & 0x0F);
    instruction.src_reg = (uint8_t)((raw_word >> 4) & 0x0F);
    instruction.imm4 = (uint8_t)(raw_word & 0x0F);
    return instruction;
}

void giga_vm_init(GigaVmState *state) {
    if (state == NULL) {
        return;
    }
    memset(state->registers, 0, sizeof(state->registers));
    state->flags_zero = 0;
    state->flags_carry = 0;
    state->flags_negative = 0;
    state->flags_overflow = 0;
    state->program_counter = 0;
    memset(state->memory, 0, sizeof(state->memory));
    state->loaded_program_words = 0;
}

int giga_vm_load_program(GigaVmState *state,
                         const uint16_t *program_words,
                         size_t word_count) {
    if (state == NULL || program_words == NULL) {
        return -1;
    }

    size_t required_bytes = word_count * 2u;
    if (required_bytes > GIGA_VM_MEMORY_SIZE) {
        return -2;
    }

    for (size_t index = 0; index < word_count; ++index) {
        uint16_t raw_word = program_words[index];
        size_t byte_address = index * 2u;
        state->memory[byte_address] = (uint8_t)(raw_word & 0xFFu);
        state->memory[byte_address + 1u] = (uint8_t)((raw_word >> 8) & 0xFFu);
    }

    state->loaded_program_words = word_count;
    state->program_counter = 0;
    return 0;
}

int giga_vm_fetch_word(const GigaVmState *state, uint16_t *out_word) {
    if (state == NULL || out_word == NULL) {
        return -1;
    }

    if (state->program_counter >= state->loaded_program_words) {
        return -2;
    }

    size_t byte_address = (size_t)state->program_counter * 2u;
    uint16_t low = state->memory[byte_address];
    uint16_t high = state->memory[byte_address + 1u];
    *out_word = (uint16_t)((high << 8) | low);
    return 0;
}

static inline uint8_t mask4(uint8_t value) {
    return (uint8_t)(value & 0x0F);
}

static void vm_update_flags(GigaVmState *state, const AluResult *alu_result) {
    state->flags_zero = alu_result->zero_flag;
    state->flags_carry = alu_result->carry_flag;
    state->flags_negative = alu_result->negative_flag;
    state->flags_overflow = alu_result->overflow_flag;
}

GigaVmResult giga_vm_step(GigaVmState *state) {
    if (state == NULL) {
        return GIGA_VM_ERROR;
    }

    uint16_t raw_word;
    if (giga_vm_fetch_word(state, &raw_word) != 0) {
        return GIGA_VM_PC_OUT_OF_RANGE;
    }

    GigaInstruction inst = giga_decode_instruction(raw_word);

    uint8_t dest_reg = inst.dest_reg & 0x07u;
    uint8_t src_reg = inst.src_reg & 0x07u;

    AluResult alu_result;

    switch (inst.opcode) {
        case GIGA_OP_NOP:
            state->program_counter++;
            break;

        case GIGA_OP_MOV:
            state->registers[dest_reg] = mask4(state->registers[src_reg]);
            state->program_counter++;
            break;

        case GIGA_OP_MOVI:
            state->registers[dest_reg] = mask4(inst.imm4);
            state->program_counter++;
            break;

        case GIGA_OP_ADD:
            alu_result = alu_add(state->registers[dest_reg], state->registers[src_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_SUB:
            alu_result = alu_sub(state->registers[dest_reg], state->registers[src_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_AND:
            alu_result = alu_and(state->registers[dest_reg], state->registers[src_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_OR:
            alu_result = alu_or(state->registers[dest_reg], state->registers[src_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_XOR:
            alu_result = alu_xor(state->registers[dest_reg], state->registers[src_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_NOT:
            alu_result = alu_not(state->registers[dest_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_SHL:
            alu_result = alu_shl(state->registers[dest_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_SHR:
            alu_result = alu_shr(state->registers[dest_reg]);
            state->registers[dest_reg] = alu_result.result;
            vm_update_flags(state, &alu_result);
            state->program_counter++;
            break;

        case GIGA_OP_LD: {
            uint8_t mem_addr = (uint8_t)((inst.src_reg << 4) | inst.imm4);
            state->registers[dest_reg] = mask4(state->memory[mem_addr]);
            state->program_counter++;
            break;
        }

        case GIGA_OP_ST: {
            uint8_t mem_addr = (uint8_t)((inst.dest_reg << 4) | inst.imm4);
            state->memory[mem_addr] = mask4(state->registers[src_reg]);
            state->program_counter++;
            break;
        }

        case GIGA_OP_JMP: {
            uint16_t target = (uint16_t)((inst.dest_reg << 8) | (inst.src_reg << 4) | inst.imm4);
            state->program_counter = target;
            break;
        }

        case GIGA_OP_HALT:
            return GIGA_VM_HALTED;

        default:
            return GIGA_VM_ERROR;
    }

    return GIGA_VM_OK;
}

GigaVmResult giga_vm_run(GigaVmState *state, size_t max_steps) {
    if (state == NULL) {
        return GIGA_VM_ERROR;
    }

    size_t steps = 0;
    GigaVmResult result;

    while (1) {
        result = giga_vm_step(state);

        if (result != GIGA_VM_OK) {
            return result;
        }

        steps++;
        if (max_steps > 0 && steps >= max_steps) {
            return GIGA_VM_OK;
        }
    }
}


