/*
 Mingus Virtual Machine
 Copyright (C) 2008 João Magalhães

 This file is part of Mingus Virtual Machine.

 Mingus Virtual Machine is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Mingus Virtual Machine is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Mingus Virtual Machine. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 João Magalhães
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

/**
 * The undefined value for any byte wide
 * value (single value).
 */
#define _UNDEFINED -127

/**
 * Enumeration defining all the possible
 * states for the mingus assembler parser.
 */
typedef enum MingusStates_e {
    NORMAL = 1,
    TOKEN,
    COMMENT
} MingusStates;

typedef struct MingusParser_t {
    FILE *output;
    size_t instructionCount;
    struct Instruction_t *instruction;
    struct Instruction_t instructions[1024];
    struct HashMap_t *labels;
} MingusParser;

#define MINGUS_MARK(FOR) MINGUS_MARK_N(FOR, 0)
#define MINGUS_MARK_BACK(FOR) MINGUS_MARK_N(FOR, 1)
#define MINGUS_MARK_N(FOR, N)\
    do {\
        FOR##Mark = pointer - N;\
    } while(0)

#define MINGUS_CALLBACK(FOR)\
    do {\
        if(bencodingEngine->settings.on##FOR) {\
            if(bencodingEngine->settings.on##FOR(&parser) != 0) {\
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
            }\
        }\
    } while(0)

#define MINGUS_CALLBACK_DATA(FOR) MINGUS_CALLBACK_DATA_N(FOR, 0)
#define MINGUS_CALLBACK_DATA_BACK(FOR) MINGUS_CALLBACK_DATA_N(FOR, 1)
#define MINGUS_CALLBACK_DATA_N(FOR, N)\
    do {\
        if(FOR##Mark) {\
            if(on##FOR(&parser, FOR##Mark, pointer - FOR##Mark - N) != 0) {\
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
            }\
            FOR##Mark = NULL;\
        }\
    } while(0)



void putCode(unsigned int instruction, FILE *file) {
    putc((instruction & 0x000000ff), file);
    putc((instruction & 0x0000ff00) >> 8, file);
    putc((instruction & 0x00ff0000) >> 16, file);
    putc((instruction & 0xff000000) >> 24, file);
}

void putBuffer(char *buffer, size_t size, FILE *file) {
    fwrite(buffer, 1, size, file);
}

ERROR_CODE ontokenEnd(struct MingusParser_t *parser, char *pointer, size_t size) {
    char *string = MALLOC(size + 1);
    memcpy(string, pointer, size);
    string[size] = '\0';

    if(string[0] == '.') {
        /* its's a section changer */
    }
    /* otherwise in case the last character in the string is a
    colon this token is considered to be a label */
    else if(string[size - 1] == ':') {
        string[size - 1] = '\0';
        setValueStringHashMap(parser->labels, string, (void *) parser->instructionCount);
    }
    /* otherwise it's considered to be an opcode reference
    and should be processed normally */
    else if(parser->instruction == NULL) {
        /* sets the current instruction pointer in the
        parser for correct execution */
        parser->instruction = &parser->instructions[parser->instructionCount];

        /* increments the number of instruction processed
        (this is the instruction counteter) */
        parser->instructionCount++;

        parser->instruction->code = 0x00000000;
        parser->instruction->opcode = UNSET_OPCODE;
        parser->instruction->arg1 = _UNDEFINED;
        parser->instruction->arg2 = _UNDEFINED;
        parser->instruction->arg3 = _UNDEFINED;
        parser->instruction->immediate = _UNDEFINED;
        parser->instruction->position = parser->instructionCount;

        if(strcmp(string, "load") == 0) {
            parser->instruction->opcode = LOAD;
            parser->instruction->code |= LOAD << 16;
        } else if(strcmp(string, "loadi") == 0) {
            parser->instruction->opcode = LOADI;
            parser->instruction->code |= LOADI << 16;
        } else if(strcmp(string, "store") == 0) {
            parser->instruction->opcode = STORE;
            parser->instruction->code |= STORE << 16;
        } else if(strcmp(string, "add") == 0) {
            parser->instruction->opcode = ADD;
            parser->instruction->code |= ADD << 16;
            parser->instruction = NULL;
        } else if(strcmp(string, "sub") == 0) {
            parser->instruction->opcode = SUB;
            parser->instruction->code |= SUB << 16;
            parser->instruction = NULL;
        } else if(strcmp(string, "pop") == 0) {
            parser->instruction->opcode = POP;
            parser->instruction->code |= POP << 16;
            parser->instruction = NULL;
        } else if(strcmp(string, "cmp") == 0) {
            parser->instruction->opcode = CMP;
            parser->instruction->code |= CMP << 16;
        } else if(strcmp(string, "jmp") == 0) {
            parser->instruction->opcode = JMP;
            parser->instruction->code |= JMP << 16;
        } else if(strcmp(string, "jmp_eq") == 0) {
            parser->instruction->opcode = JMP_EQ;
            parser->instruction->code |= JMP_EQ << 16;
        } else if(strcmp(string, "jmp_neq") == 0) {
            parser->instruction->opcode = JMP_NEQ;
            parser->instruction->code |= JMP_NEQ << 16;
        } else if(strcmp(string, "jmp_abs") == 0) {
            parser->instruction->opcode = JMP_ABS;
            parser->instruction->code |= JMP_ABS << 16;
        } else if(strcmp(string, "print") == 0)  {
            parser->instruction->opcode = PRINT;
            parser->instruction->code |= PRINT << 16;
            parser->instruction = NULL;
        } else if(strcmp(string, "halt") == 0) {
            parser->instruction->opcode = HALT;
            parser->instruction->code |= HALT << 16;
            parser->instruction = NULL;
        }
    } else {
        switch(parser->instruction->opcode) {
            case HALT:
                break;

            case LOAD:
            case LOADI:
            case STORE:
                if(parser->instruction->immediate == _UNDEFINED) {
                    parser->instruction->immediate = atoi(string);

                    parser->instruction->code |= (unsigned char) parser->instruction->immediate;

                    parser->instruction = NULL;
                }

                break;

            case JMP:
            case JMP_EQ:
                if(parser->instruction->immediate == _UNDEFINED) {
                    memcpy(parser->instruction->string, string, size + 1);
                    parser->instruction = NULL;
                }

                break;

            case ADD:
            case SUB:
                break;

            case CMP:
                if(parser->instruction->arg1 == _UNDEFINED) {
                    parser->instruction->arg1 = atoi(string);

                    parser->instruction->code |= parser->instruction->arg1 << 8;

                    parser->instruction = NULL;
                }

                break;
        }
    }

    FREE(string);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE oncommentEnd(struct MingusParser_t *parser, char *pointer, size_t size) {
    char *string = MALLOC(size + 1);
    memcpy(string, pointer, size);
    string[size] = '\0';

    FREE(string);

    /* raises no error */
    RAISE_NO_ERROR;
}






int main(int argc, const char *argv[]) {
    /* allocates space for the byte to be used
    durring the parsing of the file */
    char byte;

    size_t index;
    size_t fileSize;

    char *buffer;
    char *pointer;
    char *tokenEndMark;
    char *commentEndMark;

    struct Code_t code;
    struct MingusParser_t parser;

    FILE *out;

    /* allocates space for the file structure to
    hold the reference to be assembled */
    FILE *file;

    /* starts the parser initial state as the
    normal state in between operations */
    enum MingusStates_e state = NORMAL;

    /* counts the number of bytes in the asm file
    and then opens the asm file to be assembled in
    binary mode (required for parsing) */
    countFile("c:/calc.mia", &fileSize);
    FOPEN(&file, "c:/calc.mia", "rb");

    /* in case the file could not be read, returns
    immediately in error */
    if(file == NULL) { PRINTF("error reading file"); return 1; }

    /* tries to open the output file in writing mode,
    this is the file to old the assembled code */
    FOPEN(&out, "c:/calc.moc", "wb");

    /* in case the file could not be opened, returns
    immediately in error */
    if(out == NULL) { PRINTF("error opening output file"); return 1; }

    /* allocates the required size to read the complete file
    and then reads it completly */
    buffer = (char *) MALLOC(fileSize);
    fread(buffer, 1, fileSize, file);

    /* sets the initial pointer position to the position of
    the begining of the buffer */
    pointer = buffer;

    /* updates the parser structure setting the appropriate
    output file (buffer) and the initial opcode value */
    parser.output = out;
    parser.instruction = NULL;
    parser.instructionCount = 0;

    /* creates the hash map to hold the various labels */
    createHashMap(&parser.labels, 0);

    /* iterates continuously over the file buffer to
    parse the file and generate the output */
    while(1) {
        /* checks if the pointer did not overflow the
        current buffer itartion, in case it not retrieves
        the current byte as the byte pointed */
        if(pointer == buffer + fileSize) { break; }
        byte = *pointer;

        /* switches over the current state of the parser
        to operate accordingly over the current buffer */
        switch(state) {
            case NORMAL:
                switch(byte) {
                    case ';':
                        state = COMMENT;

                        MINGUS_MARK(commentEnd);

                        /* breaks the switch */
                        break;

                    case ' ':
                    case '\r':
                    case '\n':
                        /* breaks the switch */
                        break;

                    default:
                        state = TOKEN;

                        MINGUS_MARK(tokenEnd);

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
                        state = NORMAL;

                        MINGUS_CALLBACK_DATA(tokenEnd);

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
                        state = NORMAL;

                        MINGUS_CALLBACK_DATA(commentEnd);

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

    memcpy(code.header.magic, "MING", 4);
    code.header.version = MINGUS_CODE_VERSION;
    code.header.dataSize = 0;
    code.header.codeSize = parser.instructionCount * sizeof(int);

    putBuffer((char *) &code.header, sizeof(struct CodeHeader_t), parser.output);

    for(index = 0; index < parser.instructionCount; index++) {
        /* sets the current instruction pointer in the
        parser for correct execution */
        parser.instruction = &parser.instructions[index];

        switch(parser.instruction->opcode) {
            case JMP:
            case JMP_EQ:
                getValueStringHashMap(parser.labels, parser.instruction->string, (void **) &parser.instruction->immediate);
                parser.instruction->code |= (unsigned char) (parser.instruction->immediate - parser.instruction->position);

                break;
        }

        putCode(parser.instruction->code, parser.output);
    }

    /* prints a logging message indicating the results
    of the assembling */
    PRINTF_F("Processed %d instructions...\n", parser.instructionCount);

    /* releases the buffer, to avoid any memory leaking */
    FREE(buffer);

    /* closes both the input and output files (all the parsing
    has been done) the output has been generated */
    fclose(out);
    fclose(file);

    /* retuns with no error (normal return) */
    return 0;
}
