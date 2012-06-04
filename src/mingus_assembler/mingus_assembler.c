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
 * Enumeration defining all the possible
 * states for the mingus assembler parser.
 */
typedef enum MingusStates_e {
	NORMAL = 1,
    TOKEN,
	COMMENT
} MingusStates;

typedef enum MingusOpcodes_e {
	UNSET_OPCODE = 1,
	HALT,
    LOAD,
	ADD
} MingusOpcodes;

typedef struct MingusParser_t {
	enum MingusOpcodes_e opcode;
	int reg1;
	int reg2;
	int reg3;
	unsigned int instruction;
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
            if(on##FOR) {\
                if(on##FOR(&parser, FOR##Mark, pointer - FOR##Mark - N) != 0) {\
                    RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
                }\
            }\
            FOR##Mark = NULL;\
        }\
    } while(0)









int ontokenEnd(struct MingusParser_t *parser, unsigned char *pointer, size_t size) {
	char *string = MALLOC(size + 1);
	memcpy(string, pointer, size);
	string[size] = '\0';

	if(parser->opcode == UNSET_OPCODE) {
		parser->instruction = 0x00000000;

		if(strcmp(string, "loadi") == 0) {
			parser->opcode = LOAD;
			parser->reg1 = 0;
			parser->reg2 = 0;
			parser->reg3 = 0;

			parser->instruction |= 0x00010000;
		} else if(strcmp(string, "add") == 0) {
			parser->opcode = ADD;

			parser->instruction |= 0x00020000;
		} else if(strcmp(string, "halt") == 0) {
			parser->opcode = UNSET_OPCODE;

			parser->instruction |= 0x00000000;

			// flush instruction
		}
	} else {
		switch(parser->opcode) {
			case HALT:
				break;

			case LOAD:
				

				break;

			case ADD:
				break;
		}
	}

	printf("TOKEN -> '%s'\n", string);

	return 0;
}

int oncommentEnd(struct MingusParser_t *parser, unsigned char *pointer, size_t size) {
	char *string = MALLOC(size + 1);
	memcpy(string, pointer, size);
	string[size] = '\0';

	printf("COMMENT -> '%s'\n", string);

	return 0;
}






int main(int argc, const char *argv[]) {
	/* allocates space for the byte to be used
	durring the parsing of the file */
	char byte;


	size_t fileSize;

	char *buffer;
	char *pointer;
	char *tokenEndMark;
	char *commentEndMark;

	struct MingusParser_t parser;

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

    /**
     * iterate over all the lines in the file
     * 1. check if the line is empty (skip)
     * 2. check if the line contains a comment (ignore)
     * 3. check the name of the opcode
     * 4. retrieve the appropriate arguments to the operation
     */

	/* allocates the required size to read the complete file
	and then reads it completly */
	buffer = (char *) MALLOC(fileSize);
	fread(buffer, 1, fileSize, file);
		
	pointer = buffer;


	parser.opcode = UNSET_OPCODE;


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

	/* releases the buffer, to avoid any memory leaking */
	FREE(buffer);

	/* closes the file (all the parsing has been done) 
	the output has been generated */
    fclose(file);

    return 0;
}
