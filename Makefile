cc = gcc
rm = rm
cflags = -Wall
clibs = -lviriatum
install = install
prefix = /usr/local

all: mingus mingusa

install: all
	$(install) mingus mingusa $(prefix)/bin

clean:
	$(rm) -f mingus mingusa

mingus: src/mingus/mingus.c
	$(cc) $(cflags) $(clibs) src/mingus/mingus.c -o mingus

mingusa: src/mingus_assembler/mingus_assembler.c
	$(cc) $(cflags) $(clibs) src/mingus_assembler/mingus_assembler.c -o mingusa
