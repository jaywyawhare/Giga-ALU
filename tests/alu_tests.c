#include <stdio.h>
#include <stdint.h>
#include "alu/alu.h"

static int test_add_exhaustive(void) {
    int failure_count = 0;
    for (uint8_t left_operand = 0; left_operand < 16; ++left_operand) {
        for (uint8_t right_operand = 0; right_operand < 16; ++right_operand) {
            AluResult add_result = alu_add(left_operand, right_operand);
            uint8_t expected_sum = (uint8_t)((left_operand + right_operand) & 0x0F);
            uint8_t expected_carry =
                (uint8_t)(((left_operand + right_operand) > 0x0F) ? 1u : 0u);
            if (add_result.result != expected_sum ||
                add_result.carry_flag != expected_carry) {
                printf("ADD fail: %u + %u -> result=%u (exp %u), C=%u (exp %u)\n",
                       left_operand,
                       right_operand,
                       add_result.result,
                       expected_sum,
                       add_result.carry_flag,
                       expected_carry);
                ++failure_count;
            }
        }
    }
    return failure_count;
}

static int test_sub_basic(void) {
    int failure_count = 0;

    struct {
        uint8_t minuend;
        uint8_t subtrahend;
        uint8_t expected_result;
        uint8_t expected_carry_flag; /* 1 = no borrow */
    } subtraction_cases[] = {
        {0, 0, 0, 1},
        {5, 3, 2, 1},
        {3, 5, (uint8_t)((3 - 5) & 0x0F), 0},
        {15, 1, 14, 1},
    };

    size_t case_count = sizeof(subtraction_cases) / sizeof(subtraction_cases[0]);
    for (size_t case_index = 0; case_index < case_count; ++case_index) {
        AluResult subtraction_result =
            alu_sub(subtraction_cases[case_index].minuend,
                    subtraction_cases[case_index].subtrahend);
        if (subtraction_result.result != subtraction_cases[case_index].expected_result ||
            subtraction_result.carry_flag !=
                subtraction_cases[case_index].expected_carry_flag) {
            printf("SUB fail: %u - %u -> result=%u (exp %u), C=%u (exp %u)\n",
                   subtraction_cases[case_index].minuend,
                   subtraction_cases[case_index].subtrahend,
                   subtraction_result.result,
                   subtraction_cases[case_index].expected_result,
                   subtraction_result.carry_flag,
                   subtraction_cases[case_index].expected_carry_flag);
            ++failure_count;
        }
    }
    return failure_count;
}

static int test_logic_ops(void) {
    int failure_count = 0;

    AluResult and_result = alu_and(0xA, 0x5); /* 1010 & 0101 = 0000 */
    if (and_result.result != 0x0 || and_result.zero_flag != 1) {
        printf("AND fail\n");
        ++failure_count;
    }

    AluResult or_result = alu_or(0xA, 0x5); /* 1010 | 0101 = 1111 */
    if (or_result.result != 0xF || or_result.zero_flag != 0) {
        printf("OR fail\n");
        ++failure_count;
    }

    AluResult xor_result = alu_xor(0xA, 0x5); /* 1010 ^ 0101 = 1111 */
    if (xor_result.result != 0xF || xor_result.zero_flag != 0) {
        printf("XOR fail\n");
        ++failure_count;
    }

    AluResult not_result = alu_not(0x0); /* ~0000 & 1111 = 1111 */
    if (not_result.result != 0xF || not_result.zero_flag != 0) {
        printf("NOT fail\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_shifts(void) {
    int failure_count = 0;

    AluResult shift_left_result = alu_shl(0x9); /* 1001 << 1 = 0010, carry=1 */
    if (shift_left_result.result != 0x2 || shift_left_result.carry_flag != 1) {
        printf("SHL fail\n");
        ++failure_count;
    }

    AluResult shift_right_result = alu_shr(0x9); /* 1001 >> 1 = 0100, carry=1 */
    if (shift_right_result.result != 0x4 || shift_right_result.carry_flag != 1) {
        printf("SHR fail\n");
        ++failure_count;
    }

    return failure_count;
}

int main(void) {
    int failure_count = 0;

    failure_count += test_add_exhaustive();
    failure_count += test_sub_basic();
    failure_count += test_logic_ops();
    failure_count += test_shifts();

    if (failure_count == 0) {
        printf("ALU tests: ALL PASSED\n");
        return 0;
    }

    printf("ALU tests: %d failure(s)\n", failure_count);
    return 1;
}

