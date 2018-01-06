all: mingus mingusa

clean:
	rm -f mingus mingusa

mingus: src/mingus/mingus.c
	gcc -lviriatum src/mingus/mingus.c -o mingus

mingusa: src/mingus_assembler/mingus_assembler.c
	gcc -lviriatum src/mingus_assembler/mingus_assembler.c -o mingusa
