cc := cc
rm := rm
cflags := -Wall
clibs := -lviriatum
install := install
prefix := /usr/local
debug := 0
dflags := -D HAVE_DEBUG

base: mingus mingusa

all: base examples.build

install: all
	$(install) mingus mingusa $(prefix)/bin

clean:
	$(rm) -f mingus mingusa examples/*.mic

mingus: src/mingus/mingus.c
ifeq ($(debug),1)
	$(cc) $(cflags) $(dflags) src/mingus/mingus.c -o mingus $(clibs)
else
	$(cc) $(cflags) src/mingus/mingus.c -o mingus $(clibs)
endif

mingusa: src/mingus_assembler/mingus_assembler.c
ifeq ($(debug),1)
	$(cc) $(cflags) $(dflags) src/mingus_assembler/mingus_assembler.c -o mingusa $(clibs)
else
	$(cc) $(cflags) src/mingus_assembler/mingus_assembler.c -o mingusa $(clibs)
endif

examples.build: examples/loop.mic examples/calc.mic examples/call.mic

examples/loop.mic: mingusa examples/loop.mia
	./mingusa examples/loop.mia examples/loop.mic

examples/calc.mic: mingusa examples/calc.mia
	./mingusa examples/calc.mia examples/calc.mic

examples/call.mic: mingusa examples/call.mia
	./mingusa examples/call.mia examples/call.mic

examples.run: examples/loop.mic.run examples/calc.mic.run examples/call.mic.run

examples/loop.mic.run: mingus examples/loop.mic
	./mingus examples/loop.mic

examples/calc.mic.run: mingus examples/calc.mic
	./mingus examples/calc.mic

examples/call.mic.run: mingus examples/call.mic
	./mingus examples/call.mic
