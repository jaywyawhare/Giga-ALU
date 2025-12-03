#include "parser.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_operand(const GigaOperand *operand) {
    if (operand == NULL) {
        return;
    }
    switch (operand->operand_type) {
        case GIGA_OPERAND_NONE:
            printf("(none)");
            break;
        case GIGA_OPERAND_REGISTER:
            printf("R%u", operand->value.register_index);
            break;
        case GIGA_OPERAND_IMMEDIATE:
            printf("%u", operand->value.immediate_value);
            break;
        case GIGA_OPERAND_MEMORY:
            printf("[%u]", operand->value.memory_address);
            break;
        case GIGA_OPERAND_LABEL:
            printf("'%.*s'", (int)operand->label_name_length, operand->value.label_name);
            break;
    }
}

static void print_statement(const GigaStatement *stmt) {
    if (stmt == NULL) {
        return;
    }
    switch (stmt->statement_type) {
        case GIGA_STMT_LABEL:
            printf("%zu:%zu  LABEL     '%.*s':\n",
                   stmt->data.label.source_line,
                   stmt->data.label.source_column,
                   (int)stmt->data.label.label_name_length,
                   stmt->data.label.label_name);
            break;
        case GIGA_STMT_INSTRUCTION: {
            const GigaParsedInstruction *inst = &stmt->data.instruction;
            printf("%zu:%zu  INSTR     '%.*s'",
                   inst->source_line,
                   inst->source_column,
                   (int)inst->mnemonic_length,
                   inst->mnemonic_text);
            for (size_t i = 0; i < inst->operand_count; ++i) {
                printf(" ");
                print_operand(&inst->operands[i]);
            }
            printf("\n");
            break;
        }
        case GIGA_STMT_DIRECTIVE:
            printf("%zu:%zu  DIRECTIVE '%.*s'\n",
                   stmt->data.directive.source_line,
                   stmt->data.directive.source_column,
                   (int)stmt->data.directive.directive_name_length,
                   stmt->data.directive.directive_name);
            break;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.asm>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    size_t bytes_read = fread(buffer, 1, (size_t)file_size, file);
    fclose(file);
    buffer[bytes_read] = '\0';
    GigaLexer lexer;
    giga_lexer_init(&lexer, buffer, bytes_read);
    GigaParser parser;
    giga_parser_init(&parser, &lexer);
    int parse_result = giga_parser_parse(&parser);
    if (parse_result != 0) {
        fprintf(stderr, "Parse error at %zu:%zu: %s\n",
                parser.error_line,
                parser.error_column,
                parser.error_message ? parser.error_message : "Unknown error");
        giga_parser_free(&parser);
        free(buffer);
        return 1;
    }
    GigaStatement *stmt = parser.first_statement;
    while (stmt != NULL) {
        print_statement(stmt);
        stmt = stmt->next_statement;
    }
    giga_parser_free(&parser);
    free(buffer);
    return 0;
}

