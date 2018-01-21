cc = gcc
rm = rm
cflags = -Wall
clibs = -lviriatum
install = install
prefix = /usr/local

base: mingus mingusa

all: base examples.build

install: all
	$(install) mingus mingusa $(prefix)/bin

clean:
	$(rm) -f mingus mingusa examples/*.mic

mingus: src/mingus/mingus.c
	$(cc) $(cflags) $(clibs) src/mingus/mingus.c -o mingus

mingusa: src/mingus_assembler/mingus_assembler.c
	$(cc) $(cflags) $(clibs) src/mingus_assembler/mingus_assembler.c -o mingusa

examples.build: examples/loop.mic examples/calc.mic examples/call.mic

examples/loop.mic: mingusa examples/loop.mia
	mingusa examples/loop.mia examples/loop.mic

examples/calc.mic: mingusa examples/calc.mia
	mingusa examples/calc.mia examples/calc.mic

examples/call.mic: mingusa examples/call.mia
	mingusa examples/call.mia examples/call.mic
