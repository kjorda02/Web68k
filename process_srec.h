#include "cpu.h"

extern uint32_t entryPoint;

char* read_file(unsigned int* size);
void process_records(char* rec);
uint8_t parseHex(char* str);
uint32_t writeBytes(char* start, int len);