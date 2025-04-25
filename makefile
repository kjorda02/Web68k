CC=emcc
CFLAGS=-O0 -g -Wall -std=gnu99 -lm --no-entry

SOURCES=cpu.c process_srec.c instructions0_5.c instructions4.c instructions8_9_11.c instructions12_13_14.c instructions_misc.c util.c
INCLUDES=cpu.h process_srec.h cli_emulator.h instructions.h util.h colors.h
PROGRAMS=cpu

all: $(PROGRAMS)
	@echo -e "\033[1;32m"
	@echo "----------------------------------------------------------------------------------------------"
	@echo "                   C CODE COMPILATION COMPLETE. 68K ASSEMBLY BELOW"
	@echo "----------------------------------------------------------------------------------------------"
	@echo -e "\033[0m"
	vasm_m68k_mot prueba.X68 -m68000 -chklabels -nocase -Fsrec -L output.lst -exec=START -o "output"

$(PROGRAMS): $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@.js

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) output cli_emulator.js cli_emulator.wasm cli_emulator.html
