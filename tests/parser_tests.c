#include <stdio.h>
#include <string.h>
#include "parser/parser.h"
#include "lexer/lexer.h"

static int test_simple_instruction(void) {
    int failure_count = 0;
    const char *source = "MOVI R0, 5\n";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));
    GigaParser parser;
    giga_parser_init(&parser, &lexer);

    int parse_result = giga_parser_parse(&parser);
    if (parse_result != 0) {
        printf("PARSER fail: Parse error: %s\n", parser.error_message ? parser.error_message : "Unknown");
        ++failure_count;
        giga_parser_free(&parser);
        return failure_count;
    }

    if (parser.first_statement == NULL) {
        printf("PARSER fail: No statements parsed\n");
        ++failure_count;
        giga_parser_free(&parser);
        return failure_count;
    }

    if (parser.first_statement->statement_type != GIGA_STMT_INSTRUCTION) {
        printf("PARSER fail: Expected instruction statement\n");
        ++failure_count;
    } else {
        const GigaParsedInstruction *inst = &parser.first_statement->data.instruction;
        if (strncmp(inst->mnemonic_text, "MOVI", inst->mnemonic_length) != 0) {
            printf("PARSER fail: Expected mnemonic 'MOVI'\n");
            ++failure_count;
        }
        if (inst->operand_count != 2) {
            printf("PARSER fail: Expected 2 operands, got %zu\n", inst->operand_count);
            ++failure_count;
        }
        if (inst->operands[0].operand_type != GIGA_OPERAND_REGISTER || inst->operands[0].value.register_index != 0) {
            printf("PARSER fail: Expected first operand to be R0\n");
            ++failure_count;
        }
        if (inst->operands[1].operand_type != GIGA_OPERAND_IMMEDIATE || inst->operands[1].value.immediate_value != 5) {
            printf("PARSER fail: Expected second operand to be immediate 5\n");
            ++failure_count;
        }
    }

    giga_parser_free(&parser);
    return failure_count;
}

static int test_label(void) {
    int failure_count = 0;
    const char *source = "START:\nMOV R0, R1\n";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));
    GigaParser parser;
    giga_parser_init(&parser, &lexer);

    int parse_result = giga_parser_parse(&parser);
    if (parse_result != 0) {
        printf("PARSER fail: Parse error: %s\n", parser.error_message ? parser.error_message : "Unknown");
        ++failure_count;
        giga_parser_free(&parser);
        return failure_count;
    }

    if (parser.first_statement == NULL) {
        printf("PARSER fail: No statements parsed\n");
        ++failure_count;
        giga_parser_free(&parser);
        return failure_count;
    }

    if (parser.first_statement->statement_type != GIGA_STMT_LABEL) {
        printf("PARSER fail: Expected label statement\n");
        ++failure_count;
    } else {
        const GigaParsedLabel *label = &parser.first_statement->data.label;
        if (strncmp(label->label_name, "START", label->label_name_length) != 0) {
            printf("PARSER fail: Expected label 'START'\n");
            ++failure_count;
        }
    }

    if (parser.first_statement->next_statement == NULL) {
        printf("PARSER fail: Expected second statement\n");
        ++failure_count;
    } else {
        if (parser.first_statement->next_statement->statement_type != GIGA_STMT_INSTRUCTION) {
            printf("PARSER fail: Expected instruction after label\n");
            ++failure_count;
        }
    }

    giga_parser_free(&parser);
    return failure_count;
}

static int test_register_operands(void) {
    int failure_count = 0;
    const char *source = "ADD R0, R1\n";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));
    GigaParser parser;
    giga_parser_init(&parser, &lexer);

    int parse_result = giga_parser_parse(&parser);
    if (parse_result != 0) {
        printf("PARSER fail: Parse error\n");
        ++failure_count;
        giga_parser_free(&parser);
        return failure_count;
    }

    const GigaParsedInstruction *inst = &parser.first_statement->data.instruction;
    if (inst->operand_count != 2) {
        printf("PARSER fail: Expected 2 operands\n");
        ++failure_count;
    } else {
        if (inst->operands[0].operand_type != GIGA_OPERAND_REGISTER || inst->operands[0].value.register_index != 0) {
            printf("PARSER fail: First operand should be R0\n");
            ++failure_count;
        }
        if (inst->operands[1].operand_type != GIGA_OPERAND_REGISTER || inst->operands[1].value.register_index != 1) {
            printf("PARSER fail: Second operand should be R1\n");
            ++failure_count;
        }
    }

    giga_parser_free(&parser);
    return failure_count;
}

static int test_memory_operand(void) {
    int failure_count = 0;
    const char *source = "LD R0, [5]\n";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));
    GigaParser parser;
    giga_parser_init(&parser, &lexer);

    int parse_result = giga_parser_parse(&parser);
    if (parse_result != 0) {
        printf("PARSER fail: Parse error\n");
        ++failure_count;
        giga_parser_free(&parser);
        return failure_count;
    }

    const GigaParsedInstruction *inst = &parser.first_statement->data.instruction;
    if (inst->operand_count != 2) {
        printf("PARSER fail: Expected 2 operands\n");
        ++failure_count;
    } else {
        if (inst->operands[1].operand_type != GIGA_OPERAND_MEMORY) {
            printf("PARSER fail: Second operand should be memory\n");
            ++failure_count;
        } else if (inst->operands[1].value.memory_address != 5) {
            printf("PARSER fail: Memory address should be 5\n");
            ++failure_count;
        }
    }

    giga_parser_free(&parser);
    return failure_count;
}

int main(void) {
    int failure_count = 0;

    failure_count += test_simple_instruction();
    failure_count += test_label();
    failure_count += test_register_operands();
    failure_count += test_memory_operand();

    if (failure_count == 0) {
        printf("Parser tests: ALL PASSED\n");
        return 0;
    }

    printf("Parser tests: %d failure(s)\n", failure_count);
    return 1;
}

