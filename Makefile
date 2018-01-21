cc := cc
rm := rm
cflags := -Wall
clibs := -lviriatum
install := install
prefix := /usr/local

base: mingus mingusa

all: base examples.build

install: all
	$(install) mingus mingusa $(prefix)/bin

clean:
	$(rm) -f mingus mingusa examples/*.mic

mingus: src/mingus/mingus.c
	$(cc) $(cflags) src/mingus/mingus.c -o mingus $(clibs)

mingusa: src/mingus_assembler/mingus_assembler.c
	$(cc) $(cflags) src/mingus_assembler/mingus_assembler.c -o mingusa $(clibs)

examples.run: examples.build
	./mingus examples/loop.mic
	./mingus examples/calc.mic
	./mingus examples/call.mic

examples.build: examples/loop.mic examples/calc.mic examples/call.mic

examples/loop.mic: mingusa examples/loop.mia
	./mingusa examples/loop.mia examples/loop.mic

examples/calc.mic: mingusa examples/calc.mia
	./mingusa examples/calc.mia examples/calc.mic

examples/call.mic: mingusa examples/call.mia
	./mingusa examples/call.mia examples/call.mic
