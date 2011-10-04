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

#include <stdio.h>

/**
 * The number of existing registers in the
 * virtual machine.
 */
#define NUM_REGS 4

unsigned regs[NUM_REGS];

/**
 * The example program opcodes
 * to be executed for testing.
 */
unsigned program[] = {
    0x1064,
    0x11C8,
    0x2201,
    0x0000
};

int pc = 0;

/* fetch the next word from the program */
int fetch() {
  return program[pc++];
}

/* instruction fields */
int instrNum = 0;
int reg1 = 0;
int reg2 = 0;
int reg3 = 0;
int imm = 0;

/* decode a word */
void decode(int instr) {
  instrNum = (instr & 0xF000) >> 12;
  reg1 = (instr & 0x0F00 ) >>  8;
  reg2 = (instr & 0x00F0) >>  4;
  reg3 = (instr & 0x000F);
  imm = (instr & 0x00FF);
}

/* the VM runs until this flag becomes 0 */
int running = 1;

typedef struct State {
    int running;

} State_t;

/* evaluate the last decoded instruction */
void eval() {
    /* swtiches over the instruction number */
    switch(instrNum) {
        /* in case it's the halt instruction */
        case 0:
            printf("halt\n");

            /* unsets the runnig flag */
            running = 0;

            /* breaks the switch */
            break;

        /* in case it's the loadi instruction */
        case 1:
            printf("loadi r%d #%d\n", reg1, imm);

            /* sets the integer in the register */
            regs[reg1] = imm;

            /* breaks the switch */
            break;

        /* in case it's the add instruction */
        case 2:
            printf("add r%d r%d r%d\n", reg1, reg2, reg3);

            /* sums both registers and puts the result
            in the third register */
            regs[reg1] = regs[reg2] + regs[reg3];

            /* breaks the switch */
            break;
    }
}

void showRegisters() {
    /* allocates the index */
    int index;

    /* prints the initial line */
    printf("regs = ");

    /* iterates over all the registers */
    for(index = 0; index < NUM_REGS; index++) {
        printf("%04X ", regs[index]);
    }

    /* prints the newline */
    printf( "\n" );
}

void run() {
    /* allocates the instruction */
    int instruction;

    /* iterates while the running flag is set */
    while(running) {
        /* shows the registers */
        showRegisters();

        /* fetches the next instruction */
        instruction = fetch();

        /* decodes the instruction */
        decode(instruction);

        /* evaluates the current state */
        eval();
    }

    /* shows the registers */
    showRegisters();
}

int main(int argc, const char * argv[]) {
    /* runs the virtual machine */
    run();

    /* returns with no error */
    return 0;
}
