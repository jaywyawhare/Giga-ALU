#include "lexer/lexer.h"

#include <ctype.h>
#include <string.h>

static int giga_lexer_is_identifier_start(int character) {
    return (character == '_') || isalpha(character);
}

static int giga_lexer_is_identifier_continue(int character) {
    return (character == '_') || isalnum(character);
}

void giga_lexer_init(GigaLexer *lexer, const char *buffer, size_t buffer_length) {
    if (lexer == NULL) {
        return;
    }
    lexer->buffer = buffer;
    lexer->buffer_length = buffer_length;
    lexer->current_index = 0;
    lexer->line_number = 1;
    lexer->column_number = 1;
}

static int giga_lexer_peek(const GigaLexer *lexer) {
    if (lexer->current_index >= lexer->buffer_length) {
        return 0;
    }
    return (unsigned char)lexer->buffer[lexer->current_index];
}

static int giga_lexer_advance(GigaLexer *lexer) {
    if (lexer->current_index >= lexer->buffer_length) {
        return 0;
    }
    int character = (unsigned char)lexer->buffer[lexer->current_index++];
    if (character == '\n') {
        lexer->line_number += 1;
        lexer->column_number = 1;
    } else {
        lexer->column_number += 1;
    }
    return character;
}

static void giga_lexer_skip_spaces_and_comments(GigaLexer *lexer) {
    for (;;) {
        int character = giga_lexer_peek(lexer);
        while (character == ' ' || character == '\t' || character == '\r' || character == '\f' || character == '\v') {
            giga_lexer_advance(lexer);
            character = giga_lexer_peek(lexer);
        }

        if (character == ';') {
            while (character != 0 && character != '\n') {
                giga_lexer_advance(lexer);
                character = giga_lexer_peek(lexer);
            }
            continue;
        }
        break;
    }
}

static GigaToken giga_lexer_make_token(GigaLexer *lexer,
                                       GigaTokenKind kind,
                                       size_t start_index,
                                       size_t start_line,
                                       size_t start_column) {
    GigaToken token;
    token.kind = kind;
    token.text_begin = lexer->buffer + start_index;
    token.text_length = lexer->current_index - start_index;
    token.line_number = start_line;
    token.column_number = start_column;
    return token;
}

GigaToken giga_lexer_next_token(GigaLexer *lexer) {
    GigaToken token;
    memset(&token, 0, sizeof(token));

    if (lexer == NULL || lexer->buffer == NULL) {
        token.kind = GIGA_TOKEN_EOF;
        return token;
    }

    giga_lexer_skip_spaces_and_comments(lexer);

    size_t start_index = lexer->current_index;
    size_t start_line = lexer->line_number;
    size_t start_column = lexer->column_number;

    int character = giga_lexer_peek(lexer);
    if (character == 0 || lexer->current_index >= lexer->buffer_length) {
        token.kind = GIGA_TOKEN_EOF;
        token.text_begin = NULL;
        token.text_length = 0;
        token.line_number = lexer->line_number;
        token.column_number = lexer->column_number;
        return token;
    }

    if (character == '\n') {
        giga_lexer_advance(lexer);
        return giga_lexer_make_token(lexer,
                                     GIGA_TOKEN_NEWLINE,
                                     start_index,
                                     start_line,
                                     start_column);
    }

    if (character == ',') {
        giga_lexer_advance(lexer);
        return giga_lexer_make_token(lexer,
                                     GIGA_TOKEN_COMMA,
                                     start_index,
                                     start_line,
                                     start_column);
    }
    if (character == ':') {
        giga_lexer_advance(lexer);
        return giga_lexer_make_token(lexer,
                                     GIGA_TOKEN_COLON,
                                     start_index,
                                     start_line,
                                     start_column);
    }

    if (character == '.') {
        giga_lexer_advance(lexer);
        while (giga_lexer_is_identifier_continue(giga_lexer_peek(lexer))) {
            giga_lexer_advance(lexer);
        }
        return giga_lexer_make_token(lexer,
                                     GIGA_TOKEN_DIRECTIVE,
                                     start_index,
                                     start_line,
                                     start_column);
    }

    if (isdigit(character)) {
        giga_lexer_advance(lexer);
        if (lexer->current_index - start_index == 1 &&
            lexer->buffer[start_index] == '0') {
            int next_character = giga_lexer_peek(lexer);
            if (next_character == 'x' || next_character == 'X') {
                giga_lexer_advance(lexer);
                while (isxdigit(giga_lexer_peek(lexer))) {
                    giga_lexer_advance(lexer);
                }
                return giga_lexer_make_token(lexer,
                                             GIGA_TOKEN_NUMBER,
                                             start_index,
                                             start_line,
                                             start_column);
            }
        }
        while (isdigit(giga_lexer_peek(lexer))) {
            giga_lexer_advance(lexer);
        }
        return giga_lexer_make_token(lexer,
                                     GIGA_TOKEN_NUMBER,
                                     start_index,
                                     start_line,
                                     start_column);
    }

    if (giga_lexer_is_identifier_start(character)) {
        giga_lexer_advance(lexer);
        while (giga_lexer_is_identifier_continue(giga_lexer_peek(lexer))) {
            giga_lexer_advance(lexer);
        }
        return giga_lexer_make_token(lexer,
                                     GIGA_TOKEN_IDENTIFIER,
                                     start_index,
                                     start_line,
                                     start_column);
    }

    giga_lexer_advance(lexer);
    return giga_lexer_make_token(lexer,
                                 GIGA_TOKEN_UNKNOWN,
                                 start_index,
                                 start_line,
                                 start_column);
}


