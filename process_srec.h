#include "cpu.h"

char* read_file(char* filename);
uint32_t process_records(char* rec, CPU* cpu);
uint8_t parseHex(char* str);
uint32_t writeBytes(char* start, int len, CPU* cpu);
uint32_t load_srec(char* filename, CPU* cpu);