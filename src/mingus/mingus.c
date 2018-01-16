/*
 Mingus Virtual Machine
 Copyright (c) 2008-2018 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2018 João Magalhães
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "mingus.h"

const char operands[3][32] = { "##", "==", "!=" };

unsigned int mingus_fetch(struct state_t *state) {
    return state->program[state->pc++];
}

ERROR_CODE mingus_decode(struct state_t *state, unsigned int instruction) {
    /* populates the (current) instruction with the decoded values */
    state->instruction.code = instruction;
    state->instruction.opcode = (instruction & 0xffff0000) >> 16;
    state->instruction.arg1 = (instruction & 0x00000f00) >> 8;
    state->instruction.arg2 = (instruction & 0x000000f0) >> 4;
    state->instruction.arg3 = (instruction & 0x0000000f);
    state->instruction.immediate = (instruction & 0x000000ff);

    /* returns the control flow with no error */
    RAISE_NO_ERROR;
}

ERROR_CODE mingus_eval(struct state_t *state) {
    /* retrieves the current instruction */
    struct instruction_t *instruction = &state->instruction;

    /* allocates space for two (temporary) operands and
    for a possible result from operations over them */
    int operand1;
    int operand2;
    int result;

    /* swtiches over the instruction number */
    switch(instruction->opcode) {
        case HALT:
            V_DEBUG("halt\n");

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so == 0);

            /* unsets the running flag */
            state->running = FALSE;

            /* breaks the switch */
            break;

        case LOAD:
            V_DEBUG_F("load #%08x (#%08x)\n", instruction->immediate, state->locals[instruction->immediate]);

            /* loads the value located at the immediate location to the stack and
            increments the stack pointer by such value */
            MINGUS_PUSH(state, state->locals[instruction->immediate]);

            /* breaks the switch */
            break;

        case LOADI:
            V_DEBUG_F("loadi #%08x\n", instruction->immediate);

            /* sets the integer in the top of stack and then
            increments the current stack pointer */
            MINGUS_PUSH(state, instruction->immediate);

            /* breaks the switch */
            break;

        case STORE:
            V_DEBUG_F("store #%08x #%08x\n", instruction->immediate, MINGUS_PEEK(state));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* stores the top of the stack in the local storage */
            state->locals[instruction->immediate] = MINGUS_POP(state);

            /* breaks the switch */
            break;

        case ADD:
            V_DEBUG_F("add #%08x #%08x\n", MINGUS_PEEK(state), MINGUS_PEEK_OFF(state, 1));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 1);

            /* retrieves both operands from the stack and then pops
            both elements from it */
            operand2 = MINGUS_POP(state);
            operand1 = MINGUS_POP(state);

            /* adds the values on top of the stack and then
            sets the sum in the top of the stack */
            MINGUS_PUSH(state, operand1 + operand2);

            /* breaks the switch */
            break;

        case SUB:
            V_DEBUG_F("sub #%08x #%08x\n", MINGUS_PEEK(state), MINGUS_PEEK_OFF(state, 1));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 1);

            /* retrieves both operands from the stack and then pops
            both elements from it */
            operand2 = MINGUS_POP(state);
            operand1 = MINGUS_POP(state);

            /* subtracts the values on top of the stack and then
            sets the subtraction in the top of the stack */
            MINGUS_PUSH(state, operand1 - operand2);

            /* breaks the switch */
            break;

        case POP:
            V_DEBUG_F("pop #%08x\n", MINGUS_PEEK(state));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* pops the top element from the stack */
            MINGUS_POP_S(state);

            /* breaks the switch */
            break;

        case CMP:
            V_DEBUG_F("cmp '%s' #%08x #%08x\n", operands[instruction->arg1], MINGUS_PEEK(state), MINGUS_PEEK_OFF(state, 1));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 1);

            /* retrieves both operands from the stack and then pops
            the second element from it */
            operand2 = MINGUS_POP(state);
            operand1 = MINGUS_PEEK(state);

            /* switches over the kind of comparison that is going
            to be performed */
            switch(instruction->arg1) {
                case 1:
                    result = operand1 == operand2 ? 1 : 0;
                    break;

                case 2:
                    result = operand1 != operand2 ? 1 : 0;
                    break;
            }

            /* pushes the result of the comparison to the stack */
            MINGUS_PUSH(state, result);

            /* breaks the switch */
            break;

        case JMP:
            V_DEBUG_F("jmp %d\n", instruction->immediate);

            /* incrments the program counter with the immediate
            value of this (relative) jump operation */
            state->pc += instruction->immediate;

            /* breaks the switch */
            break;

        case JMP_EQ:
            V_DEBUG_F("jmp_eq %d #%08x\n", instruction->immediate, MINGUS_PEEK(state));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* pops the current stack top as the result value that is going
            to be used for the equality validation */
            result = MINGUS_POP(state);

            /* compares the current stack top with zero (comparision
            verified) and increments the program counter if that's the case */
            if(result == 1) {
                state->pc += instruction->immediate;
            }

            /* breaks the switch */
            break;

        case JMP_NEQ:
            V_DEBUG_F("jmp_neq %d #%08x\n", instruction->immediate, MINGUS_PEEK(state));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* pops the current stack top as the result value that is going
            to be used for the inequality validation */
            result = MINGUS_POP(state);

            /* compares the current stack top with zero (comparision
            failed) and increments the program counter if that's the case */
            if(result == 0) {
                state->pc += instruction->immediate;
            }

            /* breaks the switch */
            break;

        case JMP_ABS:
            V_DEBUG_F("jmp_abs #%08x\n", instruction->immediate);

            /* updates the program counter to the immediate
            value of the current instruction (long jump) */
            state->pc = instruction->immediate;

            /* breaks the switch */
            break;

        case CALL:
            V_DEBUG_F("call #%08x %d\n", instruction->immediate, instruction->arg1);

            /* pushes the number of arguments, the function location
            and the current program counter to the stack */
            MINGUS_CALL_PUSH(state, instruction->arg1)
            MINGUS_CALL_PUSH(state, instruction->immediate)
            MINGUS_CALL_PUSH(state, state->pc)

            /* updates the current program counter with the jump location
            for the function */
            state->pc = instruction->immediate;

            /* breaks the switch */
            break;

        case RET:
            V_DEBUG("ret\n");

            state->pc = MINGUS_CALL_POP(state);
            MINGUS_CALL_POP_S(state);
            MINGUS_CALL_POP_S(state);

            /* breaks the switch */
            break;

        case PRINT:
            V_DEBUG_F("print #%08x\n", MINGUS_PEEK(state));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* retrieves the current top value from the stack and
            prints it to the standard output */
            PRINTF_F("%d\n", state->stack[state->so - 1]);

            /* breaks the switch */
            break;

        case PRINTS:
            V_DEBUG_F("prints #%08x\n", MINGUS_PEEK(state));

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* retrieves the current top value from the stack and
            prints the string in such address to the standard output */
            PRINTF_F("%s\n", (char *) &state->stack[state->so - 1]);

            /* breaks the switch */
            break;

        default:
            RAISE_ERROR_F(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Invalid opcode '%d'",
                instruction->opcode
            );
    }

    /* raises no error as this is the final enpoint
    for the instruction evaluation */
    RAISE_NO_ERROR;
}

ERROR_CODE run(char *file_path) {
    /* allocates the value to be used to verify the
    exitence of error from the function */
    ERROR_CODE return_value;

    /* allocates the space for the instruction
    value (its a "normal" integer value, 32 bit)*/
    int instruction;

    /* allocates sapce for the variable that will
    hold the size of the bytecode buffer and for
    the buffer that will hold the bytecode */
    size_t size;
    unsigned char *buffer;

    /* allocates space for the reference to the header
    of the code object to be interpreted */
    struct code_header_t *header;

    /* creates the virtual machine state, no program
    buffer is already set (defered loading) */
    struct state_t state = { 1, 0, 0, 0, NULL };

    /* in case the provided file path is not valid raises
    and error indicating the problem */
    if(file_path == NULL) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "No input file"
        );
    }

    /* reads the program file and verifies if there was an
    error if that's the case return immediately */
    return_value = read_file(file_path, &buffer, &size);
    if(IS_ERROR_CODE(return_value)) {
        RAISE_ERROR_F(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem reading file %s",
            file_path
        );
    }

    /* sets the program buffer in the state, effectively initializing
    the virtual machine */
    header = (struct code_header_t *) buffer;
    state.program = (unsigned int *) (buffer + sizeof(struct code_header_t));
    state.running = TRUE;

    /* iterates while the running flag is set */
    while(state.running == TRUE) {
        /* shows the stack, to the default output
        buffer (standard output) */
        show_stack(&state);

        /* fetches the next instruction, decodes it into
        the intruction and then evalutates the current state */
        instruction = mingus_fetch(&state);
        return_value = mingus_decode(&state, instruction);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
        return_value = mingus_eval(&state);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    }

    /* normal returns of the function with no error */
    RAISE_NO_ERROR;
}

void show_stack(struct state_t *state) {
    /* allocates space for the index to be used
    in the iteration and the allocates space for
    the buffer for the print information */
    size_t count;
    unsigned int index;
    char buffer[1024];
    char *pointer;

    /* prints the initial line */
    count = SPRINTF(buffer, 1024, "%s", "stack => ");

    /* iterates over all the element currently
    under the stack (to print it) */
    for(index = 0; index < state->so; index++) {
        /* prints the stack information */
        pointer = &buffer[count];
        count += SPRINTF(pointer, 1024 - count, "%08x ", state->stack[index]);
    }

    /* prints the final newline into de buffer and
    then debugs the stack information */
    pointer = &buffer[count];
    SPRINTF(pointer, 1024 - count, "%s", "\n");
    V_DEBUG(buffer);
}

int main(int argc, const char *argv[]) {
    /* allocates the value to be used to verify the
    exitence of error from the function */
    ERROR_CODE return_value;

    /* allocates and starts the pointer to the path
    of the file to be interpreted, checks if the number
    of arguments is greater than one and in case it is
    updates the file path with the second argument */
    char *file_path = NULL;
    if(argc > 1) { file_path = (char *) argv[1]; }

    /* runs the virtual machine and verifies if an error
    as occured, if that's the case prints it */
    return_value = run(file_path);
    if(IS_ERROR_CODE(return_value)) {
        V_ERROR_F("Fatal error (%s)\n", (char *) GET_ERROR());
        RAISE_AGAIN(return_value);
    }

    /* returns with no error */
    return 0;
}
