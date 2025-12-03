#ifndef GIGA_LEXER_H
#define GIGA_LEXER_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Different token kinds in Giga-ALU assembly.
 */
typedef enum {
    GIGA_TOKEN_EOF = 0,
    GIGA_TOKEN_IDENTIFIER,
    GIGA_TOKEN_NUMBER,
    GIGA_TOKEN_COMMA,
    GIGA_TOKEN_COLON,
    GIGA_TOKEN_NEWLINE,
    GIGA_TOKEN_DIRECTIVE,
    GIGA_TOKEN_UNKNOWN
} GigaTokenKind;

/**
 * @brief One token produced by the lexer.
 */
typedef struct {
    GigaTokenKind kind;
    const char *text_begin;
    size_t text_length;
    size_t line_number;
    size_t column_number;
} GigaToken;

/**
 * @brief Lexer state for one source buffer.
 */
typedef struct {
    const char *buffer;
    size_t buffer_length;
    size_t current_index;
    size_t line_number;
    size_t column_number;
} GigaLexer;

/**
 * @brief Create a lexer over a given character buffer.
 *
 * @param lexer          Lexer object to initialise.
 * @param buffer         Pointer to character data.
 * @param buffer_length  Number of bytes in buffer.
 */
void giga_lexer_init(GigaLexer *lexer, const char *buffer, size_t buffer_length);

/**
 * @brief Get next token from the input.
 *
 * @param lexer  Lexer object.
 * @return Next token. When kind is GIGA_TOKEN_EOF, no more tokens.
 */
GigaToken giga_lexer_next_token(GigaLexer *lexer);

#endif /* GIGA_LEXER_H */


