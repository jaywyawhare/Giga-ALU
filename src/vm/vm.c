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

    /* Each word uses 2 bytes in memory. */
    size_t required_bytes = word_count * 2u;
    if (required_bytes > GIGA_VM_MEMORY_SIZE) {
        return -2;
    }

    for (size_t index = 0; index < word_count; ++index) {
        uint16_t raw_word = program_words[index];
        size_t byte_address = index * 2u;
        state->memory[byte_address] = (uint8_t)(raw_word & 0xFFu);         /* low byte */
        state->memory[byte_address + 1u] = (uint8_t)((raw_word >> 8) & 0xFFu); /* high byte */
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
        return -2; /* PC out of range */
    }

    size_t byte_address = (size_t)state->program_counter * 2u;
    uint16_t low = state->memory[byte_address];
    uint16_t high = state->memory[byte_address + 1u];
    *out_word = (uint16_t)((high << 8) | low);
    return 0;
}


