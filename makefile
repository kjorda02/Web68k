CC=gcc -lm
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=cpu.c process_srec.c
LIBRARIES=cpu.o
INCLUDES=cpu.h process_srec.h
PROGRAMS=process_srec
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)
	vasm_m68k_mot prueba.s -m68000 -chklabels -nocase -Fsrec -exec=START -o "output"

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) output