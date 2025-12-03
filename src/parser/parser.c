#include "parser/parser.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void giga_parser_error(GigaParser *parser, const char *message, size_t line, size_t column) {
    if (parser == NULL) {
        return;
    }
    parser->has_error = 1;
    parser->error_message = message;
    parser->error_line = line;
    parser->error_column = column;
}

static void giga_parser_advance(GigaParser *parser) {
    if (parser == NULL || parser->lexer == NULL) {
        return;
    }
    parser->current_token = parser->lookahead_token;
    parser->lookahead_token = giga_lexer_next_token(parser->lexer);
}

static int giga_parser_expect(GigaParser *parser, GigaTokenKind expected_kind) {
    if (parser == NULL) {
        return 0;
    }
    if (parser->current_token.kind != expected_kind) {
        giga_parser_error(parser, "Unexpected token", parser->current_token.line_number, parser->current_token.column_number);
        return 0;
    }
    return 1;
}

static GigaStatement *giga_parser_alloc_statement(GigaParser *parser) {
    if (parser == NULL) {
        return NULL;
    }
    GigaStatement *stmt = (GigaStatement *)calloc(1, sizeof(GigaStatement));
    if (stmt == NULL) {
        giga_parser_error(parser, "Out of memory", parser->current_token.line_number, parser->current_token.column_number);
        return NULL;
    }
    if (parser->first_statement == NULL) {
        parser->first_statement = stmt;
        parser->last_statement = stmt;
    } else {
        parser->last_statement->next_statement = stmt;
        parser->last_statement = stmt;
    }
    return stmt;
}

static int giga_parser_parse_register(GigaParser *parser, GigaOperand *operand) {
    if (parser == NULL || operand == NULL) {
        return 0;
    }
    if (parser->current_token.kind != GIGA_TOKEN_IDENTIFIER) {
        return 0;
    }
    const char *text = parser->current_token.text_begin;
    size_t length = parser->current_token.text_length;
    if (length < 2 || text[0] != 'R') {
        return 0;
    }
    if (length > 3) {
        return 0;
    }
    char reg_char = (length == 2) ? text[1] : '\0';
    if (length == 3 && text[2] != '\0') {
        return 0;
    }
    uint8_t reg_index = 0;
    if (length == 2) {
        if (reg_char >= '0' && reg_char <= '7') {
            reg_index = (uint8_t)(reg_char - '0');
        } else {
            return 0;
        }
    } else {
        return 0;
    }
    operand->operand_type = GIGA_OPERAND_REGISTER;
    operand->value.register_index = reg_index;
    giga_parser_advance(parser);
    return 1;
}

static int giga_parser_parse_number(GigaParser *parser, GigaOperand *operand) {
    if (parser == NULL || operand == NULL) {
        return 0;
    }
    if (parser->current_token.kind != GIGA_TOKEN_NUMBER) {
        return 0;
    }
    const char *text = parser->current_token.text_begin;
    size_t length = parser->current_token.text_length;
    uint8_t value = 0;
    if (length >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        for (size_t i = 2; i < length; ++i) {
            char c = text[i];
            value = (uint8_t)(value * 16);
            if (c >= '0' && c <= '9') {
                value = (uint8_t)(value + (c - '0'));
            } else if (c >= 'a' && c <= 'f') {
                value = (uint8_t)(value + (c - 'a' + 10));
            } else if (c >= 'A' && c <= 'F') {
                value = (uint8_t)(value + (c - 'A' + 10));
            } else {
                giga_parser_error(parser, "Invalid hex digit", parser->current_token.line_number, parser->current_token.column_number);
                return 0;
            }
        }
    } else {
        for (size_t i = 0; i < length; ++i) {
            char c = text[i];
            if (c < '0' || c > '9') {
                giga_parser_error(parser, "Invalid decimal digit", parser->current_token.line_number, parser->current_token.column_number);
                return 0;
            }
            value = (uint8_t)(value * 10 + (c - '0'));
        }
    }
    if (value > 15) {
        giga_parser_error(parser, "Immediate value exceeds 4 bits (max 15)", parser->current_token.line_number, parser->current_token.column_number);
        return 0;
    }
    operand->operand_type = GIGA_OPERAND_IMMEDIATE;
    operand->value.immediate_value = value;
    giga_parser_advance(parser);
    return 1;
}

static int giga_parser_parse_memory_operand(GigaParser *parser, GigaOperand *operand) {
    if (parser == NULL || operand == NULL) {
        return 0;
    }
    if (parser->current_token.kind != GIGA_TOKEN_UNKNOWN || parser->current_token.text_length != 1 || parser->current_token.text_begin[0] != '[') {
        return 0;
    }
    giga_parser_advance(parser);
    GigaOperand addr_operand = {0};
    if (!giga_parser_parse_number(parser, &addr_operand)) {
        if (!giga_parser_parse_register(parser, &addr_operand)) {
            giga_parser_error(parser, "Expected number or register in memory address", parser->current_token.line_number, parser->current_token.column_number);
            return 0;
        }
    }
    if (parser->current_token.kind != GIGA_TOKEN_UNKNOWN || parser->current_token.text_length != 1 || parser->current_token.text_begin[0] != ']') {
        giga_parser_error(parser, "Expected ']' to close memory address", parser->current_token.line_number, parser->current_token.column_number);
        return 0;
    }
    giga_parser_advance(parser);
    operand->operand_type = GIGA_OPERAND_MEMORY;
    if (addr_operand.operand_type == GIGA_OPERAND_IMMEDIATE) {
        operand->value.memory_address = addr_operand.value.immediate_value;
    } else if (addr_operand.operand_type == GIGA_OPERAND_REGISTER) {
        operand->value.memory_address = addr_operand.value.register_index;
    } else {
        operand->value.memory_address = 0;
    }
    return 1;
}

static int giga_parser_parse_label_reference(GigaParser *parser, GigaOperand *operand) {
    if (parser == NULL || operand == NULL) {
        return 0;
    }
    if (parser->current_token.kind != GIGA_TOKEN_IDENTIFIER) {
        return 0;
    }
    operand->operand_type = GIGA_OPERAND_LABEL;
    operand->value.label_name = parser->current_token.text_begin;
    operand->label_name_length = parser->current_token.text_length;
    giga_parser_advance(parser);
    return 1;
}

static int giga_parser_parse_operand(GigaParser *parser, GigaOperand *operand) {
    if (parser == NULL || operand == NULL) {
        return 0;
    }
    if (giga_parser_parse_memory_operand(parser, operand)) {
        return 1;
    }
    if (giga_parser_parse_register(parser, operand)) {
        return 1;
    }
    if (giga_parser_parse_number(parser, operand)) {
        return 1;
    }
    if (giga_parser_parse_label_reference(parser, operand)) {
        return 1;
    }
    return 0;
}

static void giga_parser_parse_instruction(GigaParser *parser) {
    if (parser == NULL) {
        return;
    }
    if (parser->current_token.kind != GIGA_TOKEN_IDENTIFIER) {
        giga_parser_error(parser, "Expected instruction mnemonic", parser->current_token.line_number, parser->current_token.column_number);
        return;
    }
    GigaStatement *stmt = giga_parser_alloc_statement(parser);
    if (stmt == NULL) {
        return;
    }
    stmt->statement_type = GIGA_STMT_INSTRUCTION;
    stmt->data.instruction.mnemonic_text = parser->current_token.text_begin;
    stmt->data.instruction.mnemonic_length = parser->current_token.text_length;
    stmt->data.instruction.source_line = parser->current_token.line_number;
    stmt->data.instruction.source_column = parser->current_token.column_number;
    stmt->data.instruction.operand_count = 0;
    giga_parser_advance(parser);
    while (parser->current_token.kind != GIGA_TOKEN_NEWLINE && parser->current_token.kind != GIGA_TOKEN_EOF) {
        if (parser->current_token.kind == GIGA_TOKEN_COMMA) {
            giga_parser_advance(parser);
        }
        if (stmt->data.instruction.operand_count >= GIGA_MAX_OPERANDS) {
            giga_parser_error(parser, "Too many operands", parser->current_token.line_number, parser->current_token.column_number);
            return;
        }
        GigaOperand operand = {0};
        if (!giga_parser_parse_operand(parser, &operand)) {
            giga_parser_error(parser, "Expected operand", parser->current_token.line_number, parser->current_token.column_number);
            return;
        }
        stmt->data.instruction.operands[stmt->data.instruction.operand_count++] = operand;
    }
    if (parser->current_token.kind == GIGA_TOKEN_NEWLINE) {
        giga_parser_advance(parser);
    }
}

static void giga_parser_parse_label(GigaParser *parser) {
    if (parser == NULL) {
        return;
    }
    if (parser->current_token.kind != GIGA_TOKEN_IDENTIFIER) {
        giga_parser_error(parser, "Expected label name", parser->current_token.line_number, parser->current_token.column_number);
        return;
    }
    GigaStatement *stmt = giga_parser_alloc_statement(parser);
    if (stmt == NULL) {
        return;
    }
    stmt->statement_type = GIGA_STMT_LABEL;
    stmt->data.label.label_name = parser->current_token.text_begin;
    stmt->data.label.label_name_length = parser->current_token.text_length;
    stmt->data.label.source_line = parser->current_token.line_number;
    stmt->data.label.source_column = parser->current_token.column_number;
    giga_parser_advance(parser);
    if (parser->current_token.kind != GIGA_TOKEN_COLON) {
        giga_parser_error(parser, "Expected ':' after label", parser->current_token.line_number, parser->current_token.column_number);
        return;
    }
    giga_parser_advance(parser);
    if (parser->current_token.kind == GIGA_TOKEN_NEWLINE) {
        giga_parser_advance(parser);
    }
}

static void giga_parser_parse_directive(GigaParser *parser) {
    if (parser == NULL) {
        return;
    }
    if (parser->current_token.kind != GIGA_TOKEN_DIRECTIVE) {
        giga_parser_error(parser, "Expected directive", parser->current_token.line_number, parser->current_token.column_number);
        return;
    }
    GigaStatement *stmt = giga_parser_alloc_statement(parser);
    if (stmt == NULL) {
        return;
    }
    stmt->statement_type = GIGA_STMT_DIRECTIVE;
    stmt->data.directive.directive_name = parser->current_token.text_begin;
    stmt->data.directive.directive_name_length = parser->current_token.text_length;
    stmt->data.directive.source_line = parser->current_token.line_number;
    stmt->data.directive.source_column = parser->current_token.column_number;
    giga_parser_advance(parser);
    while (parser->current_token.kind != GIGA_TOKEN_NEWLINE && parser->current_token.kind != GIGA_TOKEN_EOF) {
        giga_parser_advance(parser);
    }
    if (parser->current_token.kind == GIGA_TOKEN_NEWLINE) {
        giga_parser_advance(parser);
    }
}

void giga_parser_init(GigaParser *parser, GigaLexer *lexer) {
    if (parser == NULL || lexer == NULL) {
        return;
    }
    parser->lexer = lexer;
    parser->current_token.kind = GIGA_TOKEN_EOF;
    parser->lookahead_token = giga_lexer_next_token(lexer);
    parser->has_error = 0;
    parser->error_message = NULL;
    parser->error_line = 0;
    parser->error_column = 0;
    parser->first_statement = NULL;
    parser->last_statement = NULL;
    giga_parser_advance(parser);
}

int giga_parser_parse(GigaParser *parser) {
    if (parser == NULL) {
        return 1;
    }
    while (parser->current_token.kind != GIGA_TOKEN_EOF) {
        if (parser->has_error) {
            return 1;
        }
        if (parser->current_token.kind == GIGA_TOKEN_NEWLINE) {
            giga_parser_advance(parser);
            continue;
        }
        if (parser->current_token.kind == GIGA_TOKEN_DIRECTIVE) {
            giga_parser_parse_directive(parser);
        } else if (parser->current_token.kind == GIGA_TOKEN_IDENTIFIER) {
            if (parser->lookahead_token.kind == GIGA_TOKEN_COLON) {
                giga_parser_parse_label(parser);
            } else {
                giga_parser_parse_instruction(parser);
            }
        } else {
            giga_parser_error(parser, "Unexpected token at start of statement", parser->current_token.line_number, parser->current_token.column_number);
            return 1;
        }
    }
    return parser->has_error ? 1 : 0;
}

void giga_parser_free(GigaParser *parser) {
    if (parser == NULL) {
        return;
    }
    GigaStatement *stmt = parser->first_statement;
    while (stmt != NULL) {
        GigaStatement *next = stmt->next_statement;
        free(stmt);
        stmt = next;
    }
    parser->first_statement = NULL;
    parser->last_statement = NULL;
}

