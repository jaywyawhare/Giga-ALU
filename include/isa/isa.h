#ifndef GIGA_ISA_H
#define GIGA_ISA_H

#include <stdint.h>

/**
 * @brief Number of general-purpose 4-bit registers in the VM.
 */
#define GIGA_VM_REGISTER_COUNT 8

/**
 * @brief Size of VM memory in bytes.
 *
 * Each byte holds two 4-bit values; we treat memory as bytes for simplicity.
 */
#define GIGA_VM_MEMORY_SIZE 256

/**
 * @brief Opcode values for the Giga-ALU instruction set.
 *
 * Encoding uses a 16-bit instruction word:
 * [15:12] opcode, [11:8] dest_reg, [7:4] src_reg, [3:0] small immediate / extra.
 *
 * Some instructions reuse these fields differently.
 */
typedef enum {
    GIGA_OP_NOP  = 0x0,
    GIGA_OP_MOV  = 0x1, /** MOV dest_reg, src_reg */
    GIGA_OP_MOVI = 0x2, /** MOVI dest_reg, imm4 */
    GIGA_OP_ADD  = 0x3, /** ADD dest_reg, src_reg */
    GIGA_OP_SUB  = 0x4, /** SUB dest_reg, src_reg */
    GIGA_OP_AND  = 0x5, /** AND dest_reg, src_reg */
    GIGA_OP_OR   = 0x6, /** OR  dest_reg, src_reg */
    GIGA_OP_XOR  = 0x7, /** XOR dest_reg, src_reg */
    GIGA_OP_NOT  = 0x8, /** NOT dest_reg */
    GIGA_OP_SHL  = 0x9, /** SHL dest_reg */
    GIGA_OP_SHR  = 0xA, /** SHR dest_reg */
    GIGA_OP_LD   = 0xB, /** LD dest_reg */
    GIGA_OP_ST   = 0xC, /** ST src_reg */
    GIGA_OP_JMP  = 0xD, /** JMP to address */
    GIGA_OP_HALT = 0xF  /** Stop execution */
} GigaOpcode;

/**
 * @brief Decoded view of a single 16-bit instruction word.
 */
typedef struct {
    uint16_t raw;         /** raw 16-bit instruction */
    GigaOpcode opcode;    /** high nibble */
    uint8_t dest_reg;     /** bits [11:8] */
    uint8_t src_reg;      /** bits [7:4] */
    uint8_t imm4;         /** low nibble [3:0] */
} GigaInstruction;

/**
 * @brief Decode a 16-bit instruction word into fields.
 *
 * @param raw_word 16-bit encoded instruction.
 * @return Decoded instruction structure.
 */
GigaInstruction giga_decode_instruction(uint16_t raw_word);

#endif


