#include <stdio.h>
#include <string.h>
#include "vm/vm.h"
#include "isa/isa.h"

static int test_vm_init(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    for (size_t i = 0; i < GIGA_VM_REGISTER_COUNT; ++i) {
        if (state.registers[i] != 0) {
            printf("VM fail: Register R%zu should be 0 after init\n", i);
            ++failure_count;
        }
    }

    if (state.flags_zero != 0 || state.flags_carry != 0 ||
        state.flags_negative != 0 || state.flags_overflow != 0) {
        printf("VM fail: Flags should be 0 after init\n");
        ++failure_count;
    }

    if (state.program_counter != 0) {
        printf("VM fail: PC should be 0 after init\n");
        ++failure_count;
    }

    if (state.loaded_program_words != 0) {
        printf("VM fail: loaded_program_words should be 0 after init\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_load_program(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = {
        0x2005, /* MOVI R0, 5 */
        0xF000  /* HALT */
    };

    int result = giga_vm_load_program(&state, program, 2);
    if (result != 0) {
        printf("VM fail: Failed to load program\n");
        ++failure_count;
        return failure_count;
    }

    if (state.loaded_program_words != 2) {
        printf("VM fail: loaded_program_words should be 2\n");
        ++failure_count;
    }

    if (state.program_counter != 0) {
        printf("VM fail: PC should be reset to 0 after load\n");
        ++failure_count;
    }

    uint16_t fetched_word = 0;
    result = giga_vm_fetch_word(&state, &fetched_word);
    if (result != 0) {
        printf("VM fail: Failed to fetch word\n");
        ++failure_count;
    } else {
        if (fetched_word != 0x2005) {
            printf("VM fail: Fetched word should be 0x2005, got 0x%04X\n", fetched_word);
            ++failure_count;
        }
    }

    return failure_count;
}

static int test_vm_fetch_word(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = {
        0x1001, /* MOV R0, R1 */
        0x3002, /* ADD R0, R2 */
        0xF000  /* HALT */
    };

    giga_vm_load_program(&state, program, 3);

    uint16_t word = 0;
    if (giga_vm_fetch_word(&state, &word) != 0) {
        printf("VM fail: Failed to fetch first word\n");
        ++failure_count;
    } else if (word != 0x1001) {
        printf("VM fail: First word should be 0x1001, got 0x%04X\n", word);
        ++failure_count;
    }

    state.program_counter = 1;
    if (giga_vm_fetch_word(&state, &word) != 0) {
        printf("VM fail: Failed to fetch second word\n");
        ++failure_count;
    } else if (word != 0x3002) {
        printf("VM fail: Second word should be 0x3002, got 0x%04X\n", word);
        ++failure_count;
    }

    state.program_counter = 2;
    if (giga_vm_fetch_word(&state, &word) != 0) {
        printf("VM fail: Failed to fetch third word\n");
        ++failure_count;
    } else if (word != 0xF000) {
        printf("VM fail: Third word should be 0xF000, got 0x%04X\n", word);
        ++failure_count;
    }

    state.program_counter = 3;
    if (giga_vm_fetch_word(&state, &word) == 0) {
        printf("VM fail: Should fail to fetch out of range word\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_decode_instruction(void) {
    int failure_count = 0;

    GigaInstruction inst = giga_decode_instruction(0x2005);
    if (inst.opcode != GIGA_OP_MOVI) {
        printf("VM fail: Opcode should be MOVI (0x2), got 0x%X\n", inst.opcode);
        ++failure_count;
    }
    if (inst.dest_reg != 0) {
        printf("VM fail: dest_reg should be 0, got %u\n", inst.dest_reg);
        ++failure_count;
    }
    if (inst.imm4 != 5) {
        printf("VM fail: imm4 should be 5, got %u\n", inst.imm4);
        ++failure_count;
    }

    inst = giga_decode_instruction(0x3102);
    if (inst.opcode != GIGA_OP_ADD) {
        printf("VM fail: Opcode should be ADD (0x3), got 0x%X\n", inst.opcode);
        ++failure_count;
    }
    if (inst.dest_reg != 1) {
        printf("VM fail: dest_reg should be 1, got %u\n", inst.dest_reg);
        ++failure_count;
    }
    if (inst.src_reg != 0) {
        printf("VM fail: src_reg should be 0, got %u\n", inst.src_reg);
        ++failure_count;
    }
    if (inst.imm4 != 2) {
        printf("VM fail: imm4 should be 2, got %u\n", inst.imm4);
        ++failure_count;
    }

    return failure_count;
}

int main(void) {
    int failure_count = 0;

    failure_count += test_vm_init();
    failure_count += test_vm_load_program();
    failure_count += test_vm_fetch_word();
    failure_count += test_vm_decode_instruction();

    if (failure_count == 0) {
        printf("VM tests: ALL PASSED\n");
        return 0;
    }

    printf("VM tests: %d failure(s)\n", failure_count);
    return 1;
}

