all:
	vasm_m68k_mot prueba.s -m68000 -chklabels -nocase -Fsrec -exec=START -o "output"

clean:
	rm output