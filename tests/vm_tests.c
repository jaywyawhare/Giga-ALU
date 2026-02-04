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
        0x2005,
        0xF000
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
        0x1001,
        0x3002,
        0xF000
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

static int test_vm_step_movi(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2005, 0xF000 };
    giga_vm_load_program(&state, program, 2);

    GigaVmResult result = giga_vm_step(&state);
    if (result != GIGA_VM_OK) {
        printf("VM fail: MOVI step should return OK, got %d\n", result);
        ++failure_count;
    }
    if (state.registers[0] != 5) {
        printf("VM fail: R0 should be 5 after MOVI, got %u\n", state.registers[0]);
        ++failure_count;
    }
    if (state.program_counter != 1) {
        printf("VM fail: PC should be 1 after MOVI, got %u\n", state.program_counter);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_mov(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2107, 0x1010, 0xF000 };
    giga_vm_load_program(&state, program, 3);

    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 7) {
        printf("VM fail: R0 should be 7 after MOV, got %u\n", state.registers[0]);
        ++failure_count;
    }
    if (state.registers[1] != 7) {
        printf("VM fail: R1 should still be 7, got %u\n", state.registers[1]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_add(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2003, 0x2104, 0x3010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    giga_vm_step(&state);
    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 7) {
        printf("VM fail: R0 should be 7 after ADD, got %u\n", state.registers[0]);
        ++failure_count;
    }
    if (state.flags_zero != 0) {
        printf("VM fail: Zero flag should be 0 after ADD 3+4\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_sub(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2007, 0x2103, 0x4010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    giga_vm_step(&state);
    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 4) {
        printf("VM fail: R0 should be 4 after SUB, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_and(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x200F, 0x2105, 0x5010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    giga_vm_step(&state);
    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 5) {
        printf("VM fail: R0 should be 5 after AND 0xF & 0x5, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_or(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2005, 0x210A, 0x6010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    giga_vm_step(&state);
    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 15) {
        printf("VM fail: R0 should be 15 after OR 0x5 | 0xA, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_xor(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x200F, 0x2105, 0x7010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    giga_vm_step(&state);
    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 10) {
        printf("VM fail: R0 should be 10 after XOR 0xF ^ 0x5, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_not(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2005, 0x8000, 0xF000 };
    giga_vm_load_program(&state, program, 3);

    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 10) {
        printf("VM fail: R0 should be 10 after NOT 0x5, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_shl(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2003, 0x9000, 0xF000 };
    giga_vm_load_program(&state, program, 3);

    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 6) {
        printf("VM fail: R0 should be 6 after SHL 3, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_shr(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2006, 0xA000, 0xF000 };
    giga_vm_load_program(&state, program, 3);

    giga_vm_step(&state);
    giga_vm_step(&state);

    if (state.registers[0] != 3) {
        printf("VM fail: R0 should be 3 after SHR 6, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_halt(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0xF000 };
    giga_vm_load_program(&state, program, 1);

    GigaVmResult result = giga_vm_step(&state);
    if (result != GIGA_VM_HALTED) {
        printf("VM fail: HALT should return GIGA_VM_HALTED, got %d\n", result);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_nop(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x0000, 0xF000 };
    giga_vm_load_program(&state, program, 2);

    GigaVmResult result = giga_vm_step(&state);
    if (result != GIGA_VM_OK) {
        printf("VM fail: NOP should return OK, got %d\n", result);
        ++failure_count;
    }
    if (state.program_counter != 1) {
        printf("VM fail: PC should be 1 after NOP, got %u\n", state.program_counter);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_step_jmp(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0xD002, 0x0000, 0x2005, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    GigaVmResult result = giga_vm_step(&state);
    if (result != GIGA_VM_OK) {
        printf("VM fail: JMP should return OK, got %d\n", result);
        ++failure_count;
    }
    if (state.program_counter != 2) {
        printf("VM fail: PC should be 2 after JMP, got %u\n", state.program_counter);
        ++failure_count;
    }

    giga_vm_step(&state);
    if (state.registers[0] != 5) {
        printf("VM fail: R0 should be 5 after MOVI following JMP, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_run(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2001, 0x2102, 0x3010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    GigaVmResult result = giga_vm_run(&state, 100);
    if (result != GIGA_VM_HALTED) {
        printf("VM fail: run should return HALTED, got %d\n", result);
        ++failure_count;
    }
    if (state.registers[0] != 3) {
        printf("VM fail: R0 should be 3 after run, got %u\n", state.registers[0]);
        ++failure_count;
    }

    return failure_count;
}

static int test_vm_flags_zero(void) {
    int failure_count = 0;
    GigaVmState state;
    giga_vm_init(&state);

    uint16_t program[] = { 0x2005, 0x2105, 0x4010, 0xF000 };
    giga_vm_load_program(&state, program, 4);

    giga_vm_run(&state, 100);

    if (state.registers[0] != 0) {
        printf("VM fail: R0 should be 0 after 5-5, got %u\n", state.registers[0]);
        ++failure_count;
    }
    if (state.flags_zero != 1) {
        printf("VM fail: Zero flag should be 1 after 5-5, got %u\n", state.flags_zero);
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

    failure_count += test_vm_step_movi();
    failure_count += test_vm_step_mov();
    failure_count += test_vm_step_add();
    failure_count += test_vm_step_sub();
    failure_count += test_vm_step_and();
    failure_count += test_vm_step_or();
    failure_count += test_vm_step_xor();
    failure_count += test_vm_step_not();
    failure_count += test_vm_step_shl();
    failure_count += test_vm_step_shr();
    failure_count += test_vm_step_halt();
    failure_count += test_vm_step_nop();
    failure_count += test_vm_step_jmp();
    failure_count += test_vm_run();
    failure_count += test_vm_flags_zero();

    if (failure_count == 0) {
        printf("VM tests: ALL PASSED\n");
        return 0;
    }

    printf("VM tests: %d failure(s)\n", failure_count);
    return 1;
}

