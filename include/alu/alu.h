#ifndef GIGA_ALU_H
#define GIGA_ALU_H

#include <stdint.h>

/**
 * @brief Result of one 4-bit ALU operation.
 *
 * Values are stored in the low 4 bits of each @c uint8_t.
 */
typedef struct {
    uint8_t result;         /** 4-bit result value */
    uint8_t zero_flag;      /** 1 when result is zero */
    uint8_t carry_flag;     /** 1 for carry (or no borrow in SUB) */
    uint8_t negative_flag;  /** sign bit of the 4-bit result */
    uint8_t overflow_flag;  /** two's complement overflow */
} AluResult;

/**
 * @brief Add two 4-bit operands.
 *
 * 4-bit add: operand_a + operand_b.
 *
 * @param operand_a  First 4-bit value.
 * @param operand_b  Second 4-bit value.
 * @return Result and flags after addition.
 */
AluResult alu_add(uint8_t operand_a, uint8_t operand_b);

/**
 * 4-bit subtract: operand_a - operand_b.
 *
 * @param operand_a  First 4-bit value (minuend).
 * @param operand_b  Second 4-bit value (subtrahend).
 * @return Result and flags; carry_flag is 1 when no borrow.
 */
AluResult alu_sub(uint8_t operand_a, uint8_t operand_b);

/**
 * @brief Bitwise AND on two 4-bit operands.
 *
 * Bitwise AND of two 4-bit values.
 *
 * @param operand_a  First 4-bit value.
 * @param operand_b  Second 4-bit value.
 * @return Result and flags.
 */
AluResult alu_and(uint8_t operand_a, uint8_t operand_b);

/**
 * @brief Bitwise OR on two 4-bit operands.
 *
 * Bitwise OR of two 4-bit values.
 *
 * @param operand_a  First 4-bit value.
 * @param operand_b  Second 4-bit value.
 * @return Result and flags.
 */
AluResult alu_or (uint8_t operand_a, uint8_t operand_b);

/**
 * @brief Bitwise XOR on two 4-bit operands.
 *
 * Bitwise XOR of two 4-bit values.
 *
 * @param operand_a  First 4-bit value.
 * @param operand_b  Second 4-bit value.
 * @return Result and flags.
 */
AluResult alu_xor(uint8_t operand_a, uint8_t operand_b);

/**
 * @brief Bitwise NOT of a 4-bit operand.
 *
 * Bitwise NOT of one 4-bit value.
 *
 * @param operand  4-bit value.
 * @return Result and flags.
 */
AluResult alu_not(uint8_t operand);

/**
 * @brief Logical shift-left by one bit.
 *
 * Logical shift-left of one 4-bit value by 1.
 *
 * @param operand  4-bit value.
 * @return Result and flags; carry_flag holds shifted-out bit.
 */
AluResult alu_shl(uint8_t operand);

/**
 * @brief Logical shift-right by one bit.
 *
 * Logical shift-right of one 4-bit value by 1.
 *
 * @param operand  4-bit value.
 * @return Result and flags; carry_flag holds shifted-out bit.
 */
AluResult alu_shr(uint8_t operand);

#endif
