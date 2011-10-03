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

void showRegs() {
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
    /* iterates while the running flag is set */
    while(running) {
        /* shows the registers */
        showRegs();

        /* fetches the next instruction */
        int instruction = fetch();

        /* decodes the instruction */
        decode(instruction);

        /* evaluates the current state */
        eval();
    }

    /* shows the registers */
    showRegs();
}

int main(int argc, const char * argv[]) {
    /* runs the virtual machine */
    run();

    /* returns with no error */
    return 0;
}
