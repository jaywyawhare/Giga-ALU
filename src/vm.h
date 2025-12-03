#ifndef GIGA_VM_H
#define GIGA_VM_H

#include <stdint.h>
#include <stddef.h>

#include "isa.h"
#include "alu.h"

/**
 * @brief Virtual machine state for the Giga-ALU CPU.
 */
typedef struct {
    uint8_t registers[GIGA_VM_REGISTER_COUNT]; /**4-bit general registers (stored in low nibble). */
    uint8_t flags_zero;                        /**copy of ALU zero_flag */
    uint8_t flags_carry;                       /**copy of ALU carry_flag */
    uint8_t flags_negative;                    /**copy of ALU negative_flag */
    uint8_t flags_overflow;                    /**copy of ALU overflow_flag */

    uint16_t program_counter;                  /**index of next instruction word */

    uint8_t memory[GIGA_VM_MEMORY_SIZE];       /**main memory, byte addressed */
    size_t loaded_program_words;               /**number of valid instruction words loaded */
} GigaVmState;

/**
 * @brief Initialise VM state with all registers, flags and memory cleared.
 *
 * @param state VM instance.
 */
void giga_vm_init(GigaVmState *state);

/**
 * @brief Load a program into VM memory as 16-bit instruction words.
 *
 * Words are stored little-endian: low byte at even address, high byte at odd.
 *
 * @param state          VM instance.
 * @param program_words  Pointer to instruction words.
 * @param word_count     Number of 16-bit words.
 * @return 0 on success, non-zero if program does not fit.
 */
int giga_vm_load_program(GigaVmState *state,
                         const uint16_t *program_words,
                         size_t word_count);

/**
 * @brief Read a 16-bit instruction word from memory at the current PC.
 *
 * @param state VM instance.
 * @param out_word Pointer to store the fetched 16-bit word.
 * @return 0 on success, non-zero if PC is out of loaded range.
 */
int giga_vm_fetch_word(const GigaVmState *state, uint16_t *out_word);

#endif /* GIGA_VM_H */


