#ifndef GIGA_ASSEMBLER_H
#define GIGA_ASSEMBLER_H

#include "parser/parser.h"
#include "isa/isa.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Maximum number of instruction words in assembled program.
 */
#define GIGA_ASSEMBLER_MAX_WORDS 128

/**
 * @brief Assembler result containing bytecode and metadata.
 */
typedef struct {
    uint16_t *bytecode;              /** Array of 16-bit instruction words */
    size_t word_count;                /** Number of words in bytecode */
    int has_error;                    /** 1 if assembly failed */
    const char *error_message;        /** Error message if has_error is 1 */
    size_t error_line;                /** Source line of error */
    size_t error_column;              /** Source column of error */
} GigaAssemblerResult;

/**
 * @brief Assemble parsed statements into bytecode.
 *
 * Performs two passes:
 * - Pass 1: Build label table mapping label names to instruction addresses
 * - Pass 2: Encode instructions and resolve label references
 *
 * @param statements  Linked list of parsed statements (from parser).
 * @param result      Output structure to fill with bytecode and status.
 * @return 0 on success, non-zero on error. Check result->has_error.
 */
int giga_assemble(GigaStatement *statements, GigaAssemblerResult *result);

/**
 * @brief Free bytecode allocated by assembler.
 *
 * @param result  Assembler result structure.
 */
void giga_assembler_free(GigaAssemblerResult *result);

#endif /* GIGA_ASSEMBLER_H */

