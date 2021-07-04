/*
 Mingus Virtual Machine
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Mingus Virtual Machine.

 Mingus Virtual Machine is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Mingus Virtual Machine is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Mingus Virtual Machine. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 João Magalhães
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

/**
 * The undefined value for any byte wide
 * value (single value).
 */
#define _UNDEFINED -127

/* starts the memory structures */
START_MEMORY;

/**
 * Enumeration defining all the possible
 * states for the mingus assembler parser.
 */
typedef enum mingus_states_e {
    NORMAL = 1,
    TOKEN,
    COMMENT,
    STRING
} mingus_states;

/**
 * The multiple sections that exist under
 * the mingus assembly code.
 */
typedef enum mingus_sections_e {
    TEXT = 1,
    DATA
} mingus_sections;

/**
 * Primary structure to be used in the parsing
 * of the assembly input file, should contain all
 * the required pointers to be able to parse it.
 */
typedef struct mingus_parser_t {
    /**
     * The pointer to the output file structure
     * where the encoded bytecode is going to be set.
     */
    FILE *output;

    /**
     * The current state of the parser that controls
     * what's currently being parsed (eg: token, comment).
     */
    enum mingus_states_e state;

    /**
     * The kind of section that is currently being parsed
     * (eg: text, data).
     */
    enum mingus_sections_e section;

    /**
     * The counter that "counts" the number of instructions
     * that have been parsed up until a certain point, this
     * value can be used to calculate the instruction offset.
     */
    size_t instruction_count;

    /**
     * Pointer to the current intruction being parsed, so that
     * it can be completed with the proper operands.
     */
    struct instructionf_t *instruction;

    /**
     * Array that contains the complete set of instructions for
     * the program, the static based allocation of memory poses
     * a limitation on the assembled number of instructions.
     */
    struct instructionf_t instructions[1024];

    /**
     * Integer variable that control the number of data elements
     * that have bean found and stored in the data elements structure
     */
    size_t data_element_count;

    /**
     * The reference to the current data element in parsing so that its
     * attributes can be directly manipulated.
     */
    struct data_elementf_t *data_element;

    /**
     * The complete set of data elements available parsed, to be used
     * latter in the output of the file.
     */
    struct data_elementf_t data_elements[64];

    /**
     * The hash map that maps the label name (as a string) to the
     * instruction offset (memory offset).
     */
    struct hash_map_t *labels;

    struct has_map_t *elements;
} mingus_parser;

#define MINGUS_MARK(FOR) MINGUS_MARK_N(FOR, 0)
#define MINGUS_MARK_BACK(FOR) MINGUS_MARK_N(FOR, 1)
#define MINGUS_MARK_N(FOR, N)\
    do {\
        FOR##_mark = pointer - N;\
    } while(0)

#define MINGUS_CALLBACK(FOR)\
    do {\
        if(on_##FOR(&parser) != 0) {\
            RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
        }\
    } while(0)

#define MINGUS_CALLBACK_DATA(FOR) MINGUS_CALLBACK_DATA_N(FOR, 0)
#define MINGUS_CALLBACK_DATA_BACK(FOR) MINGUS_CALLBACK_DATA_N(FOR, 1)
#define MINGUS_CALLBACK_DATA_N(FOR, N)\
    do {\
        if(FOR##_mark) {\
            if(on_##FOR(&parser, FOR##_mark, pointer - FOR##_mark - N) != 0) {\
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
            }\
            FOR##_mark = NULL;\
        }\
    } while(0)

void put_code(unsigned int instruction, FILE *file) {
    putc((instruction & 0x000000ff), file);
    putc((instruction & 0x0000ff00) >> 8, file);
    putc((instruction & 0x00ff0000) >> 16, file);
    putc((instruction & 0xff000000) >> 24, file);
}

void put_buffer(char *buffer, size_t size, FILE *file) {
    fwrite(buffer, 1, size, file);
}

ERROR_CODE on_token_end(struct mingus_parser_t *parser, char *pointer, size_t size) {
    /* allocates space for the token string to be parsed and
    copies the contents from the current pointer to it */
    char *string = MALLOC(size + 1);
    memcpy(string, pointer, size);
    string[size] = '\0';

    /* in case the string starts with a dot it must represent a section
    changer and must be treated as such */
    if(string[0] == '.') {
        if(strcmp(string, ".text") == 0) {
            parser->section = TEXT;
        } else if(strcmp(string, ".data") == 0) {
            parser->section = DATA;
        } else {
            RAISE_ERROR_F(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Invalid section %s",
                string
            );
        }
    }

    else if(parser->section == DATA) {
        /* in case the token ends with a colon then this is a
        new data allocation */
        if(string[size - 1] == ':') {
            parser->data_element = &parser->data_elements[parser->data_element_count];

            parser->data_element->offset = parser->data_element_count;
            parser->data_element->type = UNSET_T;
            parser->data_element->size = 0;

            memcpy(parser->data_element->name, string, size - 1);
            parser->data_element->name[size - 1] = '\0';

            set_value_string_hash_map(parser->elements, (unsigned char *) parser->data_element->name, parser->data_element);

            /* increments the number of data elements meaning
            that a new data element has been found */
            parser->data_element_count++;
        }
        else if(parser->data_element->type == UNSET_T) {
            if(strcmp(string, "db") == 0) {
                parser->data_element->type = BYTE_T;
            } else if(strcmp(string, "dw") == 0) {
                parser->data_element->type = WORD_T;
            } else if(strcmp(string, "dd") == 0) {
                parser->data_element->type = DWORD_T;
            } else if(strcmp(string, "dq") == 0) {
                parser->data_element->type = QWORD_T;
            }
        }
    }

    /* otherwise in case the last character in the string is a
    colon this token is considered to be a label */
    else if(string[size - 1] == ':') {
        string[size - 1] = '\0';
        set_value_string_hash_map(parser->labels, (unsigned char *) string, (void *) parser->instruction_count);
        V_DEBUG_F("label '%s' #%08x\n", string, (unsigned int) parser->instruction_count);
    }

    /* otherwise it's considered to be an opcode reference
    and should be processed normally */
    else if(parser->instruction == NULL) {
        /* sets the current instruction pointer in the
        parser for correct execution */
        parser->instruction = &parser->instructions[parser->instruction_count];

        /* increments the number of instructions processed
        (this is the instruction counter) */
        parser->instruction_count++;

        /* initializes the new instruction that has been created
        with the default values (as expecteed) */
        parser->instruction->code = 0x00000000;
        parser->instruction->opcode = UNSET_OPCODE;
        parser->instruction->arg1 = _UNDEFINED;
        parser->instruction->arg2 = _UNDEFINED;
        parser->instruction->arg3 = _UNDEFINED;
        parser->instruction->immediate = _UNDEFINED;
        parser->instruction->position = parser->instruction_count;

        V_DEBUG_F("opcode '%s'\n", string);

        if(strcmp(string, "load") == 0) {
            parser->instruction->opcode = LOAD;
        } else if(strcmp(string, "loadi") == 0) {
            parser->instruction->opcode = LOADI;
        } else if(strcmp(string, "store") == 0) {
            parser->instruction->opcode = STORE;
        } else if(strcmp(string, "add") == 0) {
            parser->instruction->opcode = ADD;
            parser->instruction = NULL;
        } else if(strcmp(string, "sub") == 0) {
            parser->instruction->opcode = SUB;
            parser->instruction = NULL;
        } else if(strcmp(string, "pop") == 0) {
            parser->instruction->opcode = POP;
            parser->instruction = NULL;
        } else if(strcmp(string, "cmp") == 0) {
            parser->instruction->opcode = CMP;
        } else if(strcmp(string, "jmp") == 0) {
            parser->instruction->opcode = JMP;
        } else if(strcmp(string, "jmp_eq") == 0 || strcmp(string, "jeq") == 0) {
            parser->instruction->opcode = JMP_EQ;
        } else if(strcmp(string, "jmp_neq") == 0 || strcmp(string, "jneq") == 0) {
            parser->instruction->opcode = JMP_NEQ;
        } else if(strcmp(string, "jmp_abs") == 0 || strcmp(string, "jabs") == 0) {
            parser->instruction->opcode = JMP_ABS;
        } else if(strcmp(string, "call") == 0) {
            parser->instruction->opcode = CALL;
        } else if(strcmp(string, "ret") == 0) {
            parser->instruction->opcode = RET;
        } else if(strcmp(string, "print") == 0)  {
            parser->instruction->opcode = PRINT;
            parser->instruction = NULL;
        } else if(strcmp(string, "prints") == 0)  {
            parser->instruction->opcode = PRINTS;
            parser->instruction = NULL;
        } else if(strcmp(string, "halt") == 0) {
            parser->instruction->opcode = HALT;
            parser->instruction = NULL;
        } else {
            RAISE_ERROR_F(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Invalid opcode %s",
                string
            );
        }
    }

    /* otherwise it should be one of the operands to the processing
    of the current context, should add more information */
    else {
        switch(parser->instruction->opcode) {
            case HALT:
                break;

            case LOAD:
                if(parser->instruction->immediate == _UNDEFINED) {
                    get_value_string_hash_map(parser->elements, (unsigned char *) string, (void **) &parser->data_element);
                    parser->instruction->immediate = parser->data_element->offset;
                    parser->instruction = NULL;
                }

                break;

            case LOADI:
            case STORE:
                if(parser->instruction->immediate == _UNDEFINED) {
                    parser->instruction->immediate = atoi(string);
                    parser->instruction = NULL;
                }

                break;

            case JMP:
            case JMP_EQ:
            case JMP_NEQ:
                if(parser->instruction->immediate == _UNDEFINED) {
                    memcpy(parser->instruction->string, string, size + 1);
                    parser->instruction->immediate = atoi(string);
                    parser->instruction = NULL;
                }

                break;

            case CALL:
                if(parser->instruction->immediate == _UNDEFINED) {
                    memcpy(parser->instruction->string, string, size + 1);
                    parser->instruction->immediate = atoi(string);
                } else if(parser->instruction->arg1 == _UNDEFINED) {
                    parser->instruction->arg1 = atoi(string);
                    parser->instruction = NULL;
                }

                break;

            case ADD:
            case SUB:
                break;

            case CMP:
                if(parser->instruction->arg1 == _UNDEFINED) {
                    parser->instruction->arg1 = atoi(string);
                    parser->instruction = NULL;
                }

                break;

            default:
                break;
        }
    }

    /* releases the current string memory (avoids any
    possible memory leak */
    FREE(string);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE on_comment_end(struct mingus_parser_t *parser, char *pointer, size_t size) {
    char *string = MALLOC(size + 1);
    memcpy(string, pointer, size);
    string[size] = '\0';

    FREE(string);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE on_string_end(struct mingus_parser_t *parser, char *pointer, size_t size) {
    char *string = MALLOC(size + 1);
    memcpy(string, pointer + 1, size - 1);
    string[size - 1] = '\0';

    if(parser->section == DATA && parser->data_element) {
        parser->data_element->size = size - 1;
        memcpy(parser->data_element->value, string, size - 1);
        parser->data_element = NULL;
    }

    FREE(string);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE build_code(struct instructionf_t *instruction) {
    instruction->code = 0x00000000;
    instruction->code |= (instruction->opcode & 0x0000ffff) << 16;

    if(instruction->arg1 != _UNDEFINED) {
        instruction->code |= (instruction->arg1 & 0x0000000f) << 8;
    }

    if(instruction->arg2 != _UNDEFINED) {
        instruction->code |= (instruction->arg2 & 0x0000000f) << 4;
    }

    if(instruction->arg3 != _UNDEFINED) {
        instruction->code |= instruction->arg3 & 0x0000000f;
    }

    if(instruction->immediate != _UNDEFINED) {
        instruction->code |= instruction->immediate & 0x000000ff;
    }

    RAISE_NO_ERROR;
}

ERROR_CODE add_instruction(
    struct mingus_parser_t *parser,
    enum opcodes_e opcode,
    char arg1,
    char arg2,
    char arg3,
    char immediate
) {
    /* sets the current instruction pointer in the
    parser for correct execution */
    parser->instruction = &parser->instructions[parser->instruction_count];

    /* increments the number of instruction processed
    (this is the instruction counter) */
    parser->instruction_count++;

    /* initializes the parser's instruction code with
    the default values (no issue) */
    parser->instruction->code = 0x00000000;
    parser->instruction->opcode = opcode;
    parser->instruction->arg1 = arg1;
    parser->instruction->arg2 = arg2;
    parser->instruction->arg3 = arg3;
    parser->instruction->immediate = immediate;
    parser->instruction->position = parser->instruction_count;

    /* raises no error as no problem occurred during execution */
    RAISE_NO_ERROR;
}

ERROR_CODE run(char *file_path, char *output_path) {
    /* allocates the value to be used to verify the
    existence of error from the function */
    ERROR_CODE return_value;

    /* allocates space for the byte to be used
    durring the parsing of the file */
    char byte;

    char is_final;

    size_t index;
    size_t file_size;

    char *buffer;
    char *pointer;
    char *token_end_mark;
    char *comment_end_mark;
    char *string_end_mark;

    size_t address;
    struct instructionf_t *instruction;

    struct code_t code;

    /* creates the parser structure, considered to be
    the major one for the creation of the output code */
    struct mingus_parser_t parser;

    /* allocates space for the file structure to
    hold the reference to be assembled */
    FILE *file;

    /* allocates space for the file that is going
    to be used as the output of the bytecode */
    FILE *out;

    /* in case the provided file path is not valid raises
    and error indicating the problem */
    if(file_path == NULL) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "No input file"
        );
    }

    /* counts the number of bytes in the asm file
    and then opens the asm file to be assembled in
    binary mode (required for parsing) */
    return_value = count_file(file_path, &file_size);
    if(IS_ERROR_CODE(return_value)) {
        RAISE_ERROR_F(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem counting file %s",
            file_path
        );
    }

    /* tries to read the file contents and in case there's
    an issue with the reading raises an error */
    FOPEN(&file, file_path, "rb");
    if(file == NULL) {
        RAISE_ERROR_F(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem opening file %s",
            file_path
        );
    }

    /* tries to open the output file in writing mode,
    this is the file to old the assembled code and in
    case there's an issue with the opening raises an error*/
    FOPEN(&out, output_path, "wb");
    if(out == NULL) {
        RAISE_ERROR_F(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem opening output file %s",
            file_path
        );
    }

    /* allocates the required size to read the complete file
    and then reads it completely */
    buffer = (char *) MALLOC(file_size);
    fread(buffer, 1, file_size, file);

    /* sets the initial pointer position to the position of
    the begining of the buffer */
    pointer = buffer;

    /* updates the parser structure setting the appropriate
    output file (buffer) and the initial opcode value */
    parser.state = NORMAL;
    parser.section = TEXT;
    parser.output = out;
    parser.instruction = NULL;
    parser.instruction_count = 0;
    parser.data_element = NULL;
    parser.data_element_count = 0;

    /* creates the hash map to hold the various labels */
    create_hash_map(&parser.labels, 0);

    create_hash_map(&parser.elements, 0);

    /* unsets the is final variable as the first cycle is
    never the final one */
    is_final = FALSE;

    /* iterates continuously over the file buffer to
    parse the file and generate the output */
    while(1) {
        /* in case the is final flag as been set in the current iteration
        breaks the current loop as the EOF character has been consumed */
        if(is_final == TRUE) {
            break;
        }

        /* checks if the pointer did not overflow the
        current buffer iteration, in that case the is final
        flag must be set for break in next cycle */
        if(pointer == buffer + file_size) {
            is_final = TRUE;
        } else {
            is_final = FALSE;
        }

        /* retrieves the proper byte value taking into account if this is
        the final interation or not */
        if(is_final == TRUE) {
            byte = '\0';
        } else {
            byte = *pointer;
        }

        /* switches over the current state of the parser
        to operate accordingly over the current buffer */
        switch(parser.state) {
            case NORMAL:
                switch(byte) {
                    case ';':
                        parser.state = COMMENT;

                        MINGUS_MARK(comment_end);

                        /* breaks the switch */
                        break;

                    case '"':
                        parser.state = STRING;

                        MINGUS_MARK(string_end);

                        /* breaks the switch */
                        break;

                    case ' ':
                    case '\r':
                    case '\n':
                    case '\0':
                        /* breaks the switch */
                        break;

                    default:
                        /* sets the current parsing state as
                        token to be used in the parsing loop */
                        parser.state = TOKEN;

                        /* marks the beggining of the token
                        to be used latter in the callback */
                        MINGUS_MARK(token_end);

                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;

            case TOKEN:
                switch(byte) {
                    case ' ':
                    case '\r':
                    case '\n':
                    case '\0':
                        parser.state = NORMAL;

                        MINGUS_CALLBACK_DATA(token_end);

                        /* breaks the switch */
                        break;

                    default:
                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;

            case COMMENT:
                switch(byte) {
                    case '\r':
                    case '\n':
                    case '\0':
                        parser.state = NORMAL;

                        MINGUS_CALLBACK_DATA(comment_end);

                        /* breaks the switch */
                        break;

                    default:
                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;

            case STRING:
                switch(byte) {
                    case '"':
                        parser.state = NORMAL;

                        MINGUS_CALLBACK_DATA(string_end);

                        /* breaks the switch */
                        break;

                    default:
                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;
        }


        /* increments the current buffer pointer
        iteration end operation */
        pointer++;
    }

    /* adds the "final" halt instruction to the output so that
    the virtual machine is always returned on the final stage */
    add_instruction(&parser, HALT, _UNDEFINED, _UNDEFINED, _UNDEFINED, _UNDEFINED);

    /* iterates over the complete set of instructions to run a series
    of post-processing operations on all of them, this is especially
    important for the JMP and CALL related operations */
    for(index = 0; index < parser.instruction_count; index++) {
        instruction = &parser.instructions[index];
        switch(instruction->opcode) {
            case JMP:
            case JMP_EQ:
            case JMP_NEQ:
                get_value_string_hash_map(parser.labels, (unsigned char *) instruction->string, (void **) &address);
                if(address != (size_t) NULL) {
                    instruction->immediate = address - instruction->position;
                    instruction->code |= (unsigned char) instruction->immediate;
                }
                break;

            case JMP_ABS:
            case CALL:
                get_value_string_hash_map(parser.labels, (unsigned char *) instruction->string, (void **) &address);
                if(address != (size_t) NULL) {
                    instruction->immediate = address;
                }
                break;

            default:
                break;
        }
    }

    /* copies the magic symbol to the beginning  of the code and
    then sets a series of default values on the global header */
    memcpy(code.header.magic, "MING", 4);
    code.header.version = MINGUS_CODE_VERSION;
    code.header.data_count = parser.data_element_count;
    code.header.code_count = parser.instruction_count;
    code.header.data_size = parser.data_element_count * sizeof(struct data_elementf_t);
    code.header.code_size = parser.instruction_count * sizeof(int);

    /* retrieves the reference to the header structure and outputs it
    directly to the parser output buffer */
    put_buffer((char *) &code.header, sizeof(struct code_header_t), parser.output);

    /* iterates over the complete set of data elements to output their
    respective elements to the output buffer */
    for(index = 0; index < parser.data_element_count; index++) {
        put_buffer(
            (char *) &parser.data_elements[index],
            sizeof(struct data_elementf_t),
            parser.output
        );
    }

    /* iterates over the complete set of instructions to ouput the code
    of it into the output buffer (directly from structure) */
    for(index = 0; index < parser.instruction_count; index++) {
        /* sets the current instruction pointer in the
        parser for correct execution */
        instruction = &parser.instructions[index];
        build_code(instruction);
        put_code(instruction->code, parser.output);
    }

    /* prints a logging message indicating the results
    of the assembling, for debugging purposes */
    PRINTF_F("Processed %d data elements...\n", (int) parser.data_element_count);
    PRINTF_F("Processed %d instructions...\n", (int) parser.instruction_count);

    /* releases the buffer, to avoid any memory leaking */
    FREE(buffer);

    /* closes both the input and output files (all the parsing
    has been done) the output has been generated */
    fclose(out);
    fclose(file);

    /* retuns with no error (normal return) */
    RAISE_NO_ERROR;
}

int main(int argc, const char *argv[]) {
    /* allocates the value to be used to verify the
    existence of error from the function */
    ERROR_CODE return_value;

    /* allocates and starts the pointer to the path
    of the file to be interpreted, checks if the number
    of arguments is greater than one and in case it is
    updates the file path with the second argument */
    char *file_path = NULL;
    char *output_path = NULL;
    if(argc > 1) { file_path = (char *) argv[1]; }
    if(argc > 2) { output_path = (char *) argv[2]; }

    /* runs the virtual machine and verifies if an error
    as occurred, if that's the case prints it */
    return_value = run(file_path, output_path);
    if(IS_ERROR_CODE(return_value)) {
        V_ERROR_F("Fatal error (%s)\n", (char *) GET_ERROR());
        RAISE_AGAIN(return_value);
    }

    /* returns with no error */
    return 0;
}
