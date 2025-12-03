#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

static char *read_entire_file(const char *file_path, size_t *out_size) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc((size_t)size + 1u);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1u, (size_t)size, file);
    fclose(file);
    buffer[read_size] = '\0';

    if (out_size != NULL) {
        *out_size = read_size;
    }
    return buffer;
}

static const char *token_kind_to_string(GigaTokenKind kind) {
    switch (kind) {
    case GIGA_TOKEN_EOF:        return "EOF";
    case GIGA_TOKEN_IDENTIFIER: return "IDENT";
    case GIGA_TOKEN_NUMBER:     return "NUMBER";
    case GIGA_TOKEN_COMMA:      return "COMMA";
    case GIGA_TOKEN_COLON:      return "COLON";
    case GIGA_TOKEN_NEWLINE:    return "NEWLINE";
    case GIGA_TOKEN_DIRECTIVE:  return "DIRECTIVE";
    case GIGA_TOKEN_UNKNOWN:    return "UNKNOWN";
    default:                    return "UNKNOWN";
    }
}

int main(int argument_count, char **argument_values) {
    if (argument_count != 2) {
        fprintf(stderr, "Usage: %s <assembly-file>\n", argument_values[0]);
        return 1;
    }

    size_t file_size = 0;
    char *file_data = read_entire_file(argument_values[1], &file_size);
    if (file_data == NULL) {
        fprintf(stderr, "Failed to read file: %s\n", argument_values[1]);
        return 1;
    }

    GigaLexer lexer;
    giga_lexer_init(&lexer, file_data, file_size);

    for (;;) {
        GigaToken token = giga_lexer_next_token(&lexer);
        if (token.kind == GIGA_TOKEN_EOF) {
            printf("EOF\n");
            break;
        }

        printf("%zu:%zu  %-10s  '", token.line_number, token.column_number,
               token_kind_to_string(token.kind));
        for (size_t index = 0; index < token.text_length; ++index) {
            char ch = token.text_begin[index];
            if (ch == '\n') {
                printf("\\n");
            } else if (ch == '\t') {
                printf("\\t");
            } else {
                putchar(ch);
            }
        }
        printf("'\n");
    }

    free(file_data);
    return 0;
}


