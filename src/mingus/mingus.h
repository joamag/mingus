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

/**
 * The size of the stack to be used in the
 * runtime of the virtual machine.
 */
#define STACK_SIZE 4096

/**
 * The size of the map of local references to
 * the variables.
 */
#define LOCALS_SIZE 512

/**
 * The version of the code file currently in
 * use, any change to the structure should
 * increment this value.
 */
#define MINGUS_CODE_VERSION 1

#define MINGUS_PUSH(state, value) state->stack[state->so] = value; state->so++;
#define MINGUS_POP(state) state->stack[state->so - 1]; state->so--
#define MINGUS_POP_S(state) state->so--
#define MINGUS_PEEK(state) state->stack[state->so - 1]
#define MINGUS_PEEK_OFF(state, offset) state->stack[state->so - offset - 1]

#define MINGUS_CALL_PUSH(state, value) state->call_stack[state->cso] = value; state->cso++;
#define MINGUS_CALL_POP(state) state->call_stack[state->cso - 1]; state->cso--
#define MINGUS_CALL_POP_S(state) state->cso--
#define MINGUS_CALL_PEEK(state) state->call_stack[state->cso - 1]
#define MINGUS_CALL_PEEK_OFF(state, offset) state->call_stack[state->cso - offset - 1]

/**
 * Enumeration defining all the opcodes for
 * the various mingus operations.
 */
typedef enum opcodes_e {
    UNSET_OPCODE = -1,
    HALT,
    LOAD,
    LOADI,
    STORE,
    ADD,
    SUB,
    POP,
    CMP,
    JMP,
    JMP_EQ,
    JMP_NEQ,
    JMP_ABS,
    CALL,
    RET,
    PRINT,
    PRINTS
} opcodes;

typedef enum data_types_e {
    UNSET_T = 1,
    BYTE_T,
    WORD_T,
    DWORD_T,
    QWORD_T
} data_types;

typedef struct code_header_t {
    char magic[4];
    unsigned int version;
    unsigned int data_count;
    unsigned int code_count;
    unsigned int data_size;
    unsigned int code_size;
} code_header;

typedef struct code_t {
    struct code_header_t header;
    char *data;
    char *code;
} code;

/**
 * Structure describing a general instruction
 * for the mingus virtual machine.
 */
typedef struct instruction_t {
    int code;
    enum opcodes_e opcode;
    char arg1;
    char arg2;
    char arg3;
    char immediate;
    unsigned int position;
} instruction;

/**
 * Structure describing a full instruction (for
 * assembling) inside mingus virtual machine.
 *
 * This version of the instruction structure should
 * not be used for runtime environments to avoid
 * spending unnecessary memory.
 */
typedef struct instructionf_t {
    int code;
    enum opcodes_e opcode;
    char arg1;
    char arg2;
    char arg3;
    char immediate;
    char string[128];
    unsigned int position;
} instructionf;

/**
 * Structure describing a data element (for
 * assembling) inside mingus virtual machine.
 *
 * This version of the data element structure should
 * not be used for runtime environments to avoid
 * spending unnecessary memory.
 */
typedef struct data_elementf_t {
    enum data_types_e type;
    unsigned int size;
    char offset;
    char name[128];
    char value[128];
} data_elementf;

/**
 * Structure describing a state of the Mingus
 * virtual machine, a 32 bit based computer like
 * VM for testing purposes.
 */
typedef struct state_t {
    /**
     * Flag controlling if the virtual machine
     * is running.
     */
    unsigned char running;

    /**
     * The program counter pointer that points
     * to the next instruction to be executed.
     */
    unsigned int pc;

    /**
     * The stack index offset, that indicates the
     * next position of the stack to be populated.
     */
    unsigned int so;

    /**
     * The call stack index offset, that indicates the
     * next position of the stack to be populated.
     */
    unsigned int cso;

    /**
     * The pointer to the buffer of instruction
     * that compose the current program.
     */
    unsigned int *program;

    /**
     * The current data stack of the virtual machine,
     * this structure contains the various values on
     * which the virtual machine can operate.
     */
    unsigned int stack[STACK_SIZE];

    /**
     * The special purpose stack to be used only for calling
     * purposes. Should store things like function address
     * original program counter and number of arguments.
     */
    unsigned int call_stack[STACK_SIZE];

    /**
     * The current set of global variables that can be
     * used in the virtual machine context.
     */
    size_t globals[LOCALS_SIZE];

    /**
     * The current instruction to be executed in the
     * current virtual machine context (state).
     */
    struct instruction_t instruction;

    /**
     * The header resulting from the parsing of the
     * object file currently in read.
     */
    struct code_header_t header;

    /**
     * Pointer to the data elements section of the reading
     * buffer so that the global data values can be accessed.
     */
    struct data_elementf_t *data_elements;
} state;

/**
 * Fetches the next instruction opcode
 * and increments the program counter.
 *
 * @param state The current virtual machine state.
 * @return The next instruction opcode.
 */
unsigned int mingus_fetch(struct state_t *state);

/**
 * Decodes the given instruction, extracting
 * the various sub-components from it and placing
 * the result in global variables.
 *
 * @param state The current virtual machine state.
 * @param instruction The instruction to be decoded
 * into the global variables.
 * @return The error code on the function execution.
 */
ERROR_CODE mingus_decode(struct state_t *state, unsigned int instruction);

/**
 * Evaluates the latest decoded instruction, and
 * changes the current machine state accordingly.
 *
 * @param state The current virtual machine state.
 * @return The error code on the function execution.
 */
ERROR_CODE mingus_eval(struct state_t *state);

/**
 * Shows the state of the stack for the provided
 * state structure.
 *
 * @param state The current virtual machine state.
 */
void show_stack(struct state_t *state);
