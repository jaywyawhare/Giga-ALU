#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "assembler/assembler.h"
#include "vm/vm.h"

#define VERSION "0.1.0"

static void print_usage(const char *program_name) {
    printf("Giga-ALU (v%s) - 4-bit ALU virtual machine\n\n", VERSION);
    printf("Usage: %s [options] <file.asm>\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("  -d, --debug    Enable debug output (show execution trace)\n");
    printf("  -r, --regs     Show final register state\n");
}

static void print_version(void) {
    printf("Giga-ALU v%s\n", VERSION);
}

static char *read_file(const char *filename, size_t *out_size) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)file_size, file);
    buffer[bytes_read] = '\0';
    fclose(file);

    if (out_size != NULL) {
        *out_size = bytes_read;
    }
    return buffer;
}

static void print_registers(const GigaVmState *state) {
    printf("\nRegisters:\n");
    for (int i = 0; i < GIGA_VM_REGISTER_COUNT; i++) {
        printf("  R%d = 0x%X (%u)\n", i, state->registers[i], state->registers[i]);
    }
    printf("\nFlags:\n");
    printf("  Zero=%u  Carry=%u  Negative=%u  Overflow=%u\n",
           state->flags_zero, state->flags_carry,
           state->flags_negative, state->flags_overflow);
}

static const char *opcode_name(GigaOpcode opcode) {
    switch (opcode) {
        case GIGA_OP_NOP:  return "NOP";
        case GIGA_OP_MOV:  return "MOV";
        case GIGA_OP_MOVI: return "MOVI";
        case GIGA_OP_ADD:  return "ADD";
        case GIGA_OP_SUB:  return "SUB";
        case GIGA_OP_AND:  return "AND";
        case GIGA_OP_OR:   return "OR";
        case GIGA_OP_XOR:  return "XOR";
        case GIGA_OP_NOT:  return "NOT";
        case GIGA_OP_SHL:  return "SHL";
        case GIGA_OP_SHR:  return "SHR";
        case GIGA_OP_LD:   return "LD";
        case GIGA_OP_ST:   return "ST";
        case GIGA_OP_JMP:  return "JMP";
        case GIGA_OP_HALT: return "HALT";
        default:           return "???";
    }
}

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    int debug_mode = 0;
    int show_regs = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--regs") == 0) {
            show_regs = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            input_file = argv[i];
        }
    }

    if (input_file == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    size_t source_size;
    char *source = read_file(input_file, &source_size);
    if (source == NULL) {
        fprintf(stderr, "Error: Could not read file '%s'\n", input_file);
        return 1;
    }

    GigaLexer lexer;
    giga_lexer_init(&lexer, source, source_size);

    GigaParser parser;
    giga_parser_init(&parser, &lexer);

    if (giga_parser_parse(&parser) != 0) {
        fprintf(stderr, "Parse error at line %zu, column %zu: %s\n",
                parser.error_line, parser.error_column, parser.error_message);
        giga_parser_free(&parser);
        free(source);
        return 1;
    }

    GigaAssemblerResult asm_result;
    if (giga_assemble(parser.first_statement, &asm_result) != 0) {
        fprintf(stderr, "Assembly error at line %zu, column %zu: %s\n",
                asm_result.error_line, asm_result.error_column, asm_result.error_message);
        giga_assembler_free(&asm_result);
        giga_parser_free(&parser);
        free(source);
        return 1;
    }

    if (debug_mode) {
        printf("Assembled %zu instruction(s)\n", asm_result.word_count);
    }

    GigaVmState vm;
    giga_vm_init(&vm);

    if (giga_vm_load_program(&vm, asm_result.bytecode, asm_result.word_count) != 0) {
        fprintf(stderr, "Error: Could not load program into VM\n");
        giga_assembler_free(&asm_result);
        giga_parser_free(&parser);
        free(source);
        return 1;
    }

    if (debug_mode) {
        printf("\nExecution trace:\n");
    }

    size_t step_count = 0;
    GigaVmResult result;

    while (1) {
        if (debug_mode) {
            uint16_t word;
            if (giga_vm_fetch_word(&vm, &word) == 0) {
                GigaInstruction inst = giga_decode_instruction(word);
                printf("  [%03u] 0x%04X  %-4s  R%u, R%u, %u\n",
                       (unsigned)vm.program_counter, word,
                       opcode_name(inst.opcode),
                       inst.dest_reg, inst.src_reg, inst.imm4);
            }
        }

        result = giga_vm_step(&vm);
        step_count++;

        if (result == GIGA_VM_HALTED) {
            if (debug_mode) {
                printf("\nProgram halted after %zu step(s)\n", step_count);
            }
            break;
        } else if (result != GIGA_VM_OK) {
            fprintf(stderr, "VM error at PC=%u (step %zu)\n",
                    vm.program_counter, step_count);
            giga_assembler_free(&asm_result);
            giga_parser_free(&parser);
            free(source);
            return 1;
        }

        if (step_count > 10000) {
            fprintf(stderr, "Error: Execution limit exceeded (infinite loop?)\n");
            giga_assembler_free(&asm_result);
            giga_parser_free(&parser);
            free(source);
            return 1;
        }
    }

    if (show_regs || debug_mode) {
        print_registers(&vm);
    }

    giga_assembler_free(&asm_result);
    giga_parser_free(&parser);
    free(source);

    return 0;
}
