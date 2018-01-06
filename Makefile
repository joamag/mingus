all: mingus mingusa

clean:
	rm -f mingus mingusa

mingus: src/mingus/mingus.c
	gcc -lviriatum -lviriatum_http src/mingus/mingus.c -o mingus

mingusa: src/mingus_assembler/mingus_assembler.c
	gcc -lviriatum -lviriatum_http src/mingus_assembler/mingus_assembler.c -o mingusa
