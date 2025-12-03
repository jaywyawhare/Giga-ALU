#include <stdio.h>
#include <string.h>
#include "lexer/lexer.h"

static int test_basic_tokens(void) {
    int failure_count = 0;
    const char *source = "MOV R0, 5\nHALT";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));

    GigaToken token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "MOV", token.text_length) != 0) {
        printf("LEXER fail: Expected IDENTIFIER 'MOV'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "R0", token.text_length) != 0) {
        printf("LEXER fail: Expected IDENTIFIER 'R0'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_COMMA) {
        printf("LEXER fail: Expected COMMA\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "5", token.text_length) != 0) {
        printf("LEXER fail: Expected NUMBER '5'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NEWLINE) {
        printf("LEXER fail: Expected NEWLINE\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "HALT", token.text_length) != 0) {
        printf("LEXER fail: Expected IDENTIFIER 'HALT'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_EOF) {
        printf("LEXER fail: Expected EOF\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_numbers(void) {
    int failure_count = 0;
    const char *source = "0 5 15 0xF 0xa";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));

    GigaToken token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "0", token.text_length) != 0) {
        printf("LEXER fail: Expected NUMBER '0'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "5", token.text_length) != 0) {
        printf("LEXER fail: Expected NUMBER '5'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "15", token.text_length) != 0) {
        printf("LEXER fail: Expected NUMBER '15'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "0xF", token.text_length) != 0) {
        printf("LEXER fail: Expected NUMBER '0xF'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "0xa", token.text_length) != 0) {
        printf("LEXER fail: Expected NUMBER '0xa'\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_comments(void) {
    int failure_count = 0;
    const char *source = "MOV R0, 1 ; comment here\nADD R0, R1";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));

    GigaToken token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "MOV", token.text_length) != 0) {
        printf("LEXER fail: Comment should be skipped\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "R0", token.text_length) != 0) {
        printf("LEXER fail: Comment should be skipped\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_COMMA) {
        printf("LEXER fail: Comment should be skipped\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NUMBER || strncmp(token.text_begin, "1", token.text_length) != 0) {
        printf("LEXER fail: Comment should be skipped\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NEWLINE) {
        printf("LEXER fail: Comment should be skipped\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "ADD", token.text_length) != 0) {
        printf("LEXER fail: Comment should be skipped\n");
        ++failure_count;
    }

    return failure_count;
}

static int test_labels(void) {
    int failure_count = 0;
    const char *source = "START:\nLOOP: MOV R0, 1";
    GigaLexer lexer;
    giga_lexer_init(&lexer, source, strlen(source));

    GigaToken token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "START", token.text_length) != 0) {
        printf("LEXER fail: Expected label 'START'\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_COLON) {
        printf("LEXER fail: Expected COLON after label\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_NEWLINE) {
        printf("LEXER fail: Expected NEWLINE after label\n");
        ++failure_count;
    }

    token = giga_lexer_next_token(&lexer);
    if (token.kind != GIGA_TOKEN_IDENTIFIER || strncmp(token.text_begin, "LOOP", token.text_length) != 0) {
        printf("LEXER fail: Expected label 'LOOP'\n");
        ++failure_count;
    }

    return failure_count;
}

int main(void) {
    int failure_count = 0;

    failure_count += test_basic_tokens();
    failure_count += test_numbers();
    failure_count += test_comments();
    failure_count += test_labels();

    if (failure_count == 0) {
        printf("Lexer tests: ALL PASSED\n");
        return 0;
    }

    printf("Lexer tests: %d failure(s)\n", failure_count);
    return 1;
}

