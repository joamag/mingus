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

/**
 * The number of existing registers in the
 * virtual machine.
 */
#define NUMBER_REGISTERS 4

/**
 * Structure describing a general instruction
 * for the mingus virtual machine.
 */
typedef struct Instruction_t {
    unsigned int code;
    int reg1;
    int reg2;
    int reg3;
    int imediate;
} Instruction;

/**
 * Structure describing a state of the
 * mingus virtual machine.
 */
typedef struct State_t {
    unsigned int running;
    unsigned int pc;
    unsigned int *program;
    int registers[NUMBER_REGISTERS];
    struct Instruction_t instruction;
} State;

/**
 * Fetches the next instruction opcode
 * and increments the program counter.
 *
 * @param state The current virtual machine state.
 * @return The next instruction opcode.
 */
unsigned int mingusFetch(struct State_t *state);

/**
 * Decodes the given instruction, extracting
 * the various sub-components from it and placing
 * the result in global variables.
 *
 * @param state The current virtual machine state.
 * @param instruction The instruction to be decoded
 * into the global variables.
 */
void mingusDecode(struct State_t *state, unsigned int instruction);

/**
 * Evaluates the latest decoded instruction, and
 * changes the current machine state accordingly.
 *
 * @param state The current virtual machine state.
 */
void mingusEval(struct State_t *state);
