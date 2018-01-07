cc = gcc
rm = rm
install = install
prefix = /usr/local

all: mingus mingusa

install: all
	$(install) mingus mingusa $(prefix)/bin

clean:
	$(rm) -f mingus mingusa

mingus: src/mingus/mingus.c
	$(cc) -lviriatum src/mingus/mingus.c -o mingus

mingusa: src/mingus_assembler/mingus_assembler.c
	$(cc) -lviriatum src/mingus_assembler/mingus_assembler.c -o mingusa
