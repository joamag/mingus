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

#include "mingus.h"

const char operands[3][32] = { "##", "==", "!=" };

unsigned int mingus_fetch(struct state_t *state) {
    return state->program[state->pc++];
}

void mingus_decode(struct state_t *state, unsigned int instruction) {
    /* populates the (current) instruction with the decoded values */
    state->instruction.opcode = (instruction & 0xffff0000) >> 16;
    state->instruction.arg1 = (instruction & 0x00000f00) >> 8;
    state->instruction.arg2 = (instruction & 0x000000f0) >> 4;
    state->instruction.arg3 = (instruction & 0x0000000f);
    state->instruction.immediate = (instruction & 0x000000ff);
}

void mingus_eval(struct state_t *state) {
    /* retrieves the current instruction */
    struct instruction_t *instruction = &state->instruction;

    /* allocates space for two (temporary) operands and
    for a possible result from operations over them */
    int operand1;
    int operand2;
    int result;

    /* swtiches over the instruction number */
    switch(instruction->opcode) {
        /* in case it's the halt instruction */
        case HALT:
            V_DEBUG_F("halt\n");

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so == 0);

            /* unsets the runnig flag */
            state->running = 0;

            /* breaks the switch */
            break;

        /* in case it's the load instruction */
        case LOAD:
            V_DEBUG_F("load #%08x (#%08x)\n", instruction->immediate, state->locals[instruction->immediate]);

            state->stack[state->so] = state->locals[instruction->immediate];
            state->so++;

            /* breaks the switch */
            break;

        /* in case it's the loadi instruction */
        case LOADI:
            V_DEBUG_F("loadi #%08x\n", instruction->immediate);

            /* sets the integer in the top of stack and then
            increments the current stack pointer */
            state->stack[state->so] = instruction->immediate;
            state->so++;

            /* breaks the switch */
            break;

        /* in case it's the store instruction */
        case STORE:
            V_DEBUG_F("store #%08x #%08x\n", instruction->immediate, state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            state->locals[instruction->immediate] = state->stack[state->so - 1];
            state->so--;

            /* breaks the switch */
            break;

        /* in case it's the add instruction */
        case ADD:
            V_DEBUG_F("add #%08x #%08x\n", state->stack[state->so - 2], state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 1);

            /* retrieves both operands from the stack and then pops
            both elements from it */
            operand1 = state->stack[state->so - 2];
            operand2 = state->stack[state->so - 1];
            state->so -= 2;

            /* adds the values on top of the stack and then
            sets the sum in the top of the stack */
            state->stack[state->so] = operand1 + operand2;
            state->so++;

            /* breaks the switch */
            break;

        /* in case it's the sub instruction */
        case SUB:
            V_DEBUG_F("sub #%08x #%08x\n", state->stack[state->so - 2], state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 1);

            /* retrieves both operands from the stack and then pops
            both elements from it */
            operand1 = state->stack[state->so - 2];
            operand2 = state->stack[state->so - 1];
            state->so -= 2;

            /* adds the values on top of the stack and then
            sets the sum in the top of the stack */
            state->stack[state->so] = operand1 - operand2;
            state->so++;

            /* breaks the switch */
            break;

        /* in case it's the pop instruction */
        case POP:
            V_DEBUG_F("pop #%08x\n", state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            /* pops the top element from the stack */
            state->so--;

            /* breaks the switch */
            break;

        /* in case it's the cmp operation */
        case CMP:
            V_DEBUG_F("cmp '%s' #%08x #%08x\n", operands[instruction->arg1], state->stack[state->so - 2], state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 1);

            /* retrieves both operands from the stack and then pops
            both elements from it */
            operand1 = state->stack[state->so - 2];
            operand2 = state->stack[state->so - 1];
            state->so -= 2;

            switch(instruction->arg1) {
                case 1:
                    result = operand1 == operand2 ? 1 : 0;
                    break;

                case 2:
                    result = operand1 != operand2 ? 1 : 0;
                    break;
            }

            state->stack[state->so] = result;
            state->so++;

            /* breaks the switch */
            break;

        /* in case it's the jmp operation */
        case JMP:
            V_DEBUG_F("jmp %02x\n", instruction->immediate);

            state->pc += instruction->immediate;

            /* breaks the switch */
            break;

        /* in case it's the jmp eq operation */
        case JMP_EQ:
            V_DEBUG_F("jmp_eq %02x #%08x\n", instruction->immediate, state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            if(state->stack[state->so - 1] == 1) {
                state->pc += instruction->immediate;
            }

            /* breaks the switch */
            break;

        /* in case it's the print instruction */
        case PRINT:
            V_DEBUG_F("print #%08x\n", state->stack[state->so - 1]);

            /* verifies the condition for the instruction
            execution without any problem */
            assert(state->so > 0);

            PRINTF_F("%d\n", state->stack[state->so - 1]);
            state->so--;

            /* breaks the switch */
            break;
    }
}

void run(char *file_path) {
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
    struct state_t state = { 1, 0, 0, NULL };

    /* reads the program file and sets the program
    buffer in the state */
    read_file(file_path, &buffer, &size);
    header = (struct code_header_t *) buffer;
    state.program = (unsigned int *) (buffer + sizeof(struct code_header_t));

    /* iterates while the running flag is set */
    while(state.running) {
        /* shows the stack, to the default output
        buffer (standard output) */
        show_stack(&state);

        /* fetches the next instruction, decodes it into
        the intruction and then evalutates the current state */
        instruction = mingus_fetch(&state);
        mingus_decode(&state, instruction);
        mingus_eval(&state);
    }
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
    /* allocates and starts the pointer to the path
    of the file to be interpreted, checks if the number
    of arguments is greater than one and in case it is
    updates the file path with the second argument */
    char *file_path = NULL;
    if(argc > 1) { file_path = (char *) argv[1]; }

    /* runs the virtual machine */
    run(file_path);

    /* returns with no error */
    return 0;
}
