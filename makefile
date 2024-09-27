CC=gcc -lm
CFLAGS=-c -O0 -g -Wall -std=gnu99

SOURCES=cpu.c process_srec.c cli_emulator.c instructions0_5.c instructions4.c instructions8_9_11.c instructions12_13_14.c instructions_misc.c
LIBRARIES=cpu.o process_srec.o instructions0_5.o instructions4.o instructions8_9_11.o instructions12_13_14.o instructions_misc.o
INCLUDES=cpu.h process_srec.h cli_emulator.h instructions.h
PROGRAMS=cli_emulator
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)
	@echo "----------------------------------------------------------------------------------------------"
	@echo "                   C CODE COMPILATION COMPLETE. 68K ASSEMBLY BELOW"
	@echo "----------------------------------------------------------------------------------------------"
	vasm_m68k_mot prueba.X68 -m68000 -chklabels -nocase -Fsrec -exec=START -o "output"

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) output