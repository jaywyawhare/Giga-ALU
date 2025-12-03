#ifndef GIGA_PARSER_H
#define GIGA_PARSER_H

#include "lexer.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Types of operands in an instruction.
 */
typedef enum {
    GIGA_OPERAND_NONE,
    GIGA_OPERAND_REGISTER,      /** Register like R0-R7 */
    GIGA_OPERAND_IMMEDIATE,     /** 4-bit immediate value */
    GIGA_OPERAND_MEMORY,        /** Memory address [addr] */
    GIGA_OPERAND_LABEL          /** Label reference for jumps */
} GigaOperandType;

/**
 * @brief One operand in an instruction.
 */
typedef struct {
    GigaOperandType operand_type;
    union {
        uint8_t register_index;     /** For GIGA_OPERAND_REGISTER */
        uint8_t immediate_value;    /** For GIGA_OPERAND_IMMEDIATE */
        uint8_t memory_address;     /** For GIGA_OPERAND_MEMORY */
        const char *label_name;     /** For GIGA_OPERAND_LABEL (points into token text) */
    } value;
    size_t label_name_length;       /** Length of label name if operand_type is LABEL */
} GigaOperand;

/**
 * @brief Maximum number of operands per instruction.
 */
#define GIGA_MAX_OPERANDS 3

/**
 * @brief One parsed instruction statement.
 */
typedef struct {
    const char *mnemonic_text;      /** Points into original source */
    size_t mnemonic_length;
    size_t operand_count;
    GigaOperand operands[GIGA_MAX_OPERANDS];
    size_t source_line;             /** Line number in source */
    size_t source_column;           /** Column number in source */
} GigaParsedInstruction;

/**
 * @brief One parsed label definition.
 */
typedef struct {
    const char *label_name;         /** Points into original source */
    size_t label_name_length;
    size_t source_line;
    size_t source_column;
} GigaParsedLabel;

/**
 * @brief Types of statements in a parsed program.
 */
typedef enum {
    GIGA_STMT_LABEL,
    GIGA_STMT_INSTRUCTION,
    GIGA_STMT_DIRECTIVE
} GigaStatementType;

/**
 * @brief One statement in a parsed program.
 */
typedef struct GigaStatement {
    GigaStatementType statement_type;
    union {
        GigaParsedLabel label;
        GigaParsedInstruction instruction;
        struct {
            const char *directive_name;
            size_t directive_name_length;
            size_t source_line;
            size_t source_column;
        } directive;
    } data;
    struct GigaStatement *next_statement;  /** Linked list */
} GigaStatement;

/**
 * @brief Parser state and result.
 */
typedef struct {
    GigaLexer *lexer;
    GigaToken current_token;
    GigaToken lookahead_token;
    int has_error;
    const char *error_message;
    size_t error_line;
    size_t error_column;
    GigaStatement *first_statement;
    GigaStatement *last_statement;
} GigaParser;

/**
 * @brief Initialise a parser with a lexer.
 *
 * @param parser  Parser object to initialise.
 * @param lexer   Lexer object (must already be initialised).
 */
void giga_parser_init(GigaParser *parser, GigaLexer *lexer);

/**
 * @brief Parse entire source into a list of statements.
 *
 * @param parser  Parser object.
 * @return 0 on success, non-zero on error. Check parser->has_error.
 */
int giga_parser_parse(GigaParser *parser);

/**
 * @brief Free all statements allocated by the parser.
 *
 * @param parser  Parser object.
 */
void giga_parser_free(GigaParser *parser);

#endif

