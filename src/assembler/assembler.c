#include "assembler/assembler.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Label table entry mapping name to instruction address.
 */
typedef struct LabelEntry {
    const char *name;
    size_t name_length;
    uint16_t address;  /* instruction word index */
    struct LabelEntry *next;
} LabelEntry;

static LabelEntry *label_table = NULL;

static void label_table_add(const char *name, size_t name_length, uint16_t address) {
    LabelEntry *entry = (LabelEntry *)calloc(1, sizeof(LabelEntry));
    if (entry == NULL) {
        return;
    }
    entry->name = name;
    entry->name_length = name_length;
    entry->address = address;
    entry->next = label_table;
    label_table = entry;
}

static uint16_t label_table_find(const char *name, size_t name_length) {
    LabelEntry *entry = label_table;
    while (entry != NULL) {
        if (entry->name_length == name_length &&
            strncmp(entry->name, name, name_length) == 0) {
            return entry->address;
        }
        entry = entry->next;
    }
    return 0xFFFF; /* not found */
}

static void label_table_free(void) {
    LabelEntry *entry = label_table;
    while (entry != NULL) {
        LabelEntry *next = entry->next;
        free(entry);
        entry = next;
    }
    label_table = NULL;
}

static int mnemonic_to_opcode(const char *mnemonic, size_t length, GigaOpcode *out_opcode) {
    if (mnemonic == NULL || out_opcode == NULL) {
        return 0;
    }

    if (length == 3 && strncmp(mnemonic, "NOP", 3) == 0) {
        *out_opcode = GIGA_OP_NOP;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "MOV", 3) == 0) {
        *out_opcode = GIGA_OP_MOV;
        return 1;
    }
    if (length == 4 && strncmp(mnemonic, "MOVI", 4) == 0) {
        *out_opcode = GIGA_OP_MOVI;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "ADD", 3) == 0) {
        *out_opcode = GIGA_OP_ADD;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "SUB", 3) == 0) {
        *out_opcode = GIGA_OP_SUB;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "AND", 3) == 0) {
        *out_opcode = GIGA_OP_AND;
        return 1;
    }
    if (length == 2 && strncmp(mnemonic, "OR", 2) == 0) {
        *out_opcode = GIGA_OP_OR;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "XOR", 3) == 0) {
        *out_opcode = GIGA_OP_XOR;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "NOT", 3) == 0) {
        *out_opcode = GIGA_OP_NOT;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "SHL", 3) == 0) {
        *out_opcode = GIGA_OP_SHL;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "SHR", 3) == 0) {
        *out_opcode = GIGA_OP_SHR;
        return 1;
    }
    if (length == 2 && strncmp(mnemonic, "LD", 2) == 0) {
        *out_opcode = GIGA_OP_LD;
        return 1;
    }
    if (length == 2 && strncmp(mnemonic, "ST", 2) == 0) {
        *out_opcode = GIGA_OP_ST;
        return 1;
    }
    if (length == 3 && strncmp(mnemonic, "JMP", 3) == 0) {
        *out_opcode = GIGA_OP_JMP;
        return 1;
    }
    if (length == 4 && strncmp(mnemonic, "HALT", 4) == 0) {
        *out_opcode = GIGA_OP_HALT;
        return 1;
    }

    return 0;
}

static void assembler_error(GigaAssemblerResult *result, const char *message, size_t line, size_t column) {
    if (result == NULL) {
        return;
    }
    result->has_error = 1;
    result->error_message = message;
    result->error_line = line;
    result->error_column = column;
}

static uint16_t encode_instruction(GigaOpcode opcode, uint8_t dest_reg, uint8_t src_reg, uint8_t imm4) {
    return (uint16_t)(((uint16_t)opcode << 12) |
                      ((uint16_t)dest_reg << 8) |
                      ((uint16_t)src_reg << 4) |
                      (uint16_t)imm4);
}

static int assemble_pass1(GigaStatement *statements, GigaAssemblerResult *result) {
    uint16_t instruction_address = 0;
    GigaStatement *stmt = statements;

    while (stmt != NULL) {
        if (stmt->statement_type == GIGA_STMT_LABEL) {
            const GigaParsedLabel *label = &stmt->data.label;
            label_table_add(label->label_name, label->label_name_length, instruction_address);
        } else if (stmt->statement_type == GIGA_STMT_INSTRUCTION) {
            instruction_address++;
            if (instruction_address >= GIGA_ASSEMBLER_MAX_WORDS) {
                assembler_error(result, "Program too large", stmt->data.instruction.source_line, stmt->data.instruction.source_column);
                return 1;
            }
        }
        stmt = stmt->next_statement;
    }

    return 0;
}

static int assemble_pass2(GigaStatement *statements, GigaAssemblerResult *result) {
    result->bytecode = (uint16_t *)calloc(GIGA_ASSEMBLER_MAX_WORDS, sizeof(uint16_t));
    if (result->bytecode == NULL) {
        assembler_error(result, "Out of memory", 0, 0);
        return 1;
    }

    result->word_count = 0;
    GigaStatement *stmt = statements;

    while (stmt != NULL) {
        if (stmt->statement_type == GIGA_STMT_INSTRUCTION) {
            const GigaParsedInstruction *inst = &stmt->data.instruction;
            GigaOpcode opcode;
            if (!mnemonic_to_opcode(inst->mnemonic_text, inst->mnemonic_length, &opcode)) {
                assembler_error(result, "Unknown mnemonic", inst->source_line, inst->source_column);
                return 1;
            }

            uint8_t dest_reg = 0;
            uint8_t src_reg = 0;
            uint8_t imm4 = 0;

            switch (opcode) {
                case GIGA_OP_NOP:
                    break;

                case GIGA_OP_MOVI:
                    if (inst->operand_count < 2) {
                        assembler_error(result, "MOVI requires 2 operands", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[0].operand_type != GIGA_OPERAND_REGISTER) {
                        assembler_error(result, "MOVI first operand must be register", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[1].operand_type != GIGA_OPERAND_IMMEDIATE) {
                        assembler_error(result, "MOVI second operand must be immediate", inst->source_line, inst->source_column);
                        return 1;
                    }
                    dest_reg = inst->operands[0].value.register_index;
                    imm4 = inst->operands[1].value.immediate_value;
                    break;

                case GIGA_OP_MOV:
                case GIGA_OP_ADD:
                case GIGA_OP_SUB:
                case GIGA_OP_AND:
                case GIGA_OP_OR:
                case GIGA_OP_XOR:
                    if (inst->operand_count < 2) {
                        assembler_error(result, "Instruction requires 2 operands", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[0].operand_type != GIGA_OPERAND_REGISTER ||
                        inst->operands[1].operand_type != GIGA_OPERAND_REGISTER) {
                        assembler_error(result, "Operands must be registers", inst->source_line, inst->source_column);
                        return 1;
                    }
                    dest_reg = inst->operands[0].value.register_index;
                    src_reg = inst->operands[1].value.register_index;
                    break;

                case GIGA_OP_NOT:
                case GIGA_OP_SHL:
                case GIGA_OP_SHR:
                    if (inst->operand_count < 1) {
                        assembler_error(result, "Instruction requires 1 operand", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[0].operand_type != GIGA_OPERAND_REGISTER) {
                        assembler_error(result, "Operand must be register", inst->source_line, inst->source_column);
                        return 1;
                    }
                    dest_reg = inst->operands[0].value.register_index;
                    break;

                case GIGA_OP_LD:
                    if (inst->operand_count < 2) {
                        assembler_error(result, "LD requires 2 operands", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[0].operand_type != GIGA_OPERAND_REGISTER) {
                        assembler_error(result, "LD first operand must be register", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[1].operand_type != GIGA_OPERAND_MEMORY) {
                        assembler_error(result, "LD second operand must be memory address", inst->source_line, inst->source_column);
                        return 1;
                    }
                    dest_reg = inst->operands[0].value.register_index;
                    imm4 = inst->operands[1].value.memory_address & 0x0F;
                    src_reg = (inst->operands[1].value.memory_address >> 4) & 0x0F;
                    break;

                case GIGA_OP_ST:
                    if (inst->operand_count < 2) {
                        assembler_error(result, "ST requires 2 operands", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[0].operand_type != GIGA_OPERAND_MEMORY) {
                        assembler_error(result, "ST first operand must be memory address", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[1].operand_type != GIGA_OPERAND_REGISTER) {
                        assembler_error(result, "ST second operand must be register", inst->source_line, inst->source_column);
                        return 1;
                    }
                    src_reg = inst->operands[1].value.register_index;
                    imm4 = inst->operands[0].value.memory_address & 0x0F;
                    dest_reg = (inst->operands[0].value.memory_address >> 4) & 0x0F;
                    break;

                case GIGA_OP_JMP:
                    if (inst->operand_count < 1) {
                        assembler_error(result, "JMP requires 1 operand", inst->source_line, inst->source_column);
                        return 1;
                    }
                    if (inst->operands[0].operand_type == GIGA_OPERAND_LABEL) {
                        uint16_t target = label_table_find(inst->operands[0].value.label_name,
                                                           inst->operands[0].label_name_length);
                        if (target == 0xFFFF) {
                            assembler_error(result, "Undefined label", inst->source_line, inst->source_column);
                            return 1;
                        }
                        dest_reg = (target >> 8) & 0x0F;
                        src_reg = (target >> 4) & 0x0F;
                        imm4 = target & 0x0F;
                    } else if (inst->operands[0].operand_type == GIGA_OPERAND_IMMEDIATE) {
                        uint16_t target = inst->operands[0].value.immediate_value;
                        dest_reg = (target >> 8) & 0x0F;
                        src_reg = (target >> 4) & 0x0F;
                        imm4 = target & 0x0F;
                    } else {
                        assembler_error(result, "JMP operand must be label or immediate", inst->source_line, inst->source_column);
                        return 1;
                    }
                    break;

                case GIGA_OP_HALT:
                    break;

                default:
                    assembler_error(result, "Unsupported opcode", inst->source_line, inst->source_column);
                    return 1;
            }

            result->bytecode[result->word_count++] = encode_instruction(opcode, dest_reg, src_reg, imm4);
        }

        stmt = stmt->next_statement;
    }

    return 0;
}

int giga_assemble(GigaStatement *statements, GigaAssemblerResult *result) {
    if (statements == NULL || result == NULL) {
        return 1;
    }

    result->bytecode = NULL;
    result->word_count = 0;
    result->has_error = 0;
    result->error_message = NULL;
    result->error_line = 0;
    result->error_column = 0;

    label_table_free();

    if (assemble_pass1(statements, result) != 0) {
        return 1;
    }

    if (assemble_pass2(statements, result) != 0) {
        if (result->bytecode != NULL) {
            free(result->bytecode);
            result->bytecode = NULL;
        }
        return 1;
    }

    return 0;
}

void giga_assembler_free(GigaAssemblerResult *result) {
    if (result == NULL) {
        return;
    }
    if (result->bytecode != NULL) {
        free(result->bytecode);
        result->bytecode = NULL;
    }
    result->word_count = 0;
    label_table_free();
}

