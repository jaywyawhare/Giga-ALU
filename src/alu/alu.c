#include "alu/alu.h"

static inline uint8_t mask4(uint8_t value) {
    return (uint8_t)(value & 0x0F);
}

static inline uint8_t sign4(uint8_t value) {
    return (uint8_t)((value >> 3) & 0x01);
}

static inline void set_zero_and_negative_flags(AluResult *result) {
    result->zero_flag = (mask4(result->result) == 0) ? 1u : 0u;
    result->negative_flag = sign4(result->result);
}

AluResult alu_add(uint8_t operand_a, uint8_t operand_b) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    uint8_t operand_a_4bit = mask4(operand_a);
    uint8_t operand_b_4bit = mask4(operand_b);
    uint8_t raw_sum = (uint8_t)(operand_a_4bit + operand_b_4bit); /* may use 5th bit as carry */

    operation_result.result = mask4(raw_sum);
    operation_result.carry_flag = (raw_sum & 0x10u) ? 1u : 0u;

    uint8_t sign_a = sign4(operand_a_4bit);
    uint8_t sign_b = sign4(operand_b_4bit);
    uint8_t sign_result = sign4(operation_result.result);
    operation_result.overflow_flag = ((sign_a == sign_b) && (sign_a != sign_result)) ? 1u : 0u;

    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_sub(uint8_t operand_a, uint8_t operand_b) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    uint8_t operand_a_4bit = mask4(operand_a);
    uint8_t operand_b_4bit = mask4(operand_b);

    /* perform 5-bit subtraction to detect borrow */
    int16_t raw_difference = (int16_t)operand_a_4bit - (int16_t)operand_b_4bit;
    if (raw_difference < 0) {
        operation_result.carry_flag = 0u; /* borrow occurred */
        raw_difference += 32; /* wrap in 5 bits */
    } else {
        operation_result.carry_flag = 1u; /* no borrow */
    }

    operation_result.result = mask4((uint8_t)raw_difference);

    uint8_t sign_a = sign4(operand_a_4bit);
    uint8_t sign_b = sign4(operand_b_4bit);
    uint8_t sign_result = sign4(operation_result.result);
    /* overflow on subtraction when signs of a and b differ and result sign differs from a */
    operation_result.overflow_flag = ((sign_a != sign_b) && (sign_a != sign_result)) ? 1u : 0u;

    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_and(uint8_t operand_a, uint8_t operand_b) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    operation_result.result = mask4(mask4(operand_a) & mask4(operand_b));
    operation_result.carry_flag = 0u;
    operation_result.overflow_flag = 0u;
    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_or(uint8_t operand_a, uint8_t operand_b) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    operation_result.result = mask4(mask4(operand_a) | mask4(operand_b));
    operation_result.carry_flag = 0u;
    operation_result.overflow_flag = 0u;
    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_xor(uint8_t operand_a, uint8_t operand_b) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    operation_result.result = mask4(mask4(operand_a) ^ mask4(operand_b));
    operation_result.carry_flag = 0u;
    operation_result.overflow_flag = 0u;
    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_not(uint8_t operand) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    operation_result.result = mask4((uint8_t)~mask4(operand));
    operation_result.carry_flag = 0u;
    operation_result.overflow_flag = 0u;
    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_shl(uint8_t operand) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    uint8_t operand_4bit = mask4(operand);
    operation_result.carry_flag = (uint8_t)((operand_4bit >> 3) & 0x01u); /* bit shifted out */
    operation_result.result = mask4((uint8_t)(operand_4bit << 1));
    operation_result.overflow_flag = 0u;
    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

AluResult alu_shr(uint8_t operand) {
    AluResult operation_result = {0, 0, 0, 0, 0};
    uint8_t operand_4bit = mask4(operand);
    operation_result.carry_flag = (uint8_t)(operand_4bit & 0x01u); /* bit shifted out */
    operation_result.result = mask4((uint8_t)(operand_4bit >> 1));
    operation_result.overflow_flag = 0u;
    set_zero_and_negative_flags(&operation_result);
    return operation_result;
}

