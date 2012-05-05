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

/**
 * The example program opcodes
 * to be executed for testing.
 */
unsigned int program[] = {
    0x1064,
    0x11C8,
    0x2201,
    0x0000
};

unsigned int mingusFetch(struct State_t *state) {
    return state->program[state->pc++];
}

void mingusDecode(struct State_t *state, unsigned int instruction) {
    /* populates the (current) instruction with the decoded values */
    state->instruction.code = (instruction & 0xF000) >> 12;
    state->instruction.reg1 = (instruction & 0x0F00) >> 8;
    state->instruction.reg2 = (instruction & 0x00F0) >> 4;
    state->instruction.reg3 = (instruction & 0x000F);
    state->instruction.imediate = (instruction & 0x00FF);
}

void mingusEval(struct State_t *state) {
    /* retrieves the current instruction */
    struct Instruction_t *instruction = &state->instruction;

    /* swtiches over the instruction number */
    switch(instruction->code) {
        /* in case it's the halt instruction */
        case 0:
            printf("halt\n");

            /* unsets the runnig flag */
            state->running = 0;

            /* breaks the switch */
            break;

        /* in case it's the loadi instruction */
        case 1:
            printf("loadi r%d #%d\n", instruction->reg1, instruction->imediate);

            /* sets the integer in the register */
            state->registers[instruction->reg1] = instruction->imediate;

            /* breaks the switch */
            break;

        /* in case it's the add instruction */
        case 2:
            printf("add r%d r%d r%d\n", instruction->reg1, instruction->reg2, instruction->reg3);

            /* sums both registers and puts the result
            in the third register */
            state->registers[instruction->reg1] = state->registers[instruction->reg2] + state->registers[instruction->reg3];

            /* breaks the switch */
            break;
    }
}

void showRegisters(struct State_t *state) {
    /* allocates the index */
    int index;

    /* prints the initial line */
    printf("regs = ");

    /* iterates over all the registers */
    for(index = 0; index < NUMBER_REGISTERS; index++) {
        /* prints the register information */
        printf("%04X ", state->registers[index]);
    }

    /* prints the newline */
    printf( "\n" );
}

void run() {
    /* allocates the instruction */
    int instruction;

    /* creates the virtual machine state */
    struct State_t state = { 1, 0, program };

    /* iterates while the running flag is set */
    while(state.running) {
        /* shows the registers */
        showRegisters(&state);

        /* fetches the next instruction */
        instruction = mingusFetch(&state);

        /* decodes the instruction */
        mingusDecode(&state, instruction);

        /* evaluates the current state */
        mingusEval(&state);
    }

    /* shows the registers */
    showRegisters(&state);
}

int main(int argc, const char *argv[]) {
    /* runs the virtual machine */
    run();

    /* returns with no error */
    return 0;
}
