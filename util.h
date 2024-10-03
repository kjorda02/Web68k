#include "instructions.h"

#define INFO 0
#define WARNING 1
#define ERROR -1
void logmsg(int8_t status, char* func, char* message);
uint8_t size_to_bytes(uint8_t size);
uint8_t check_overflow(uint32_t a, uint32_t b, uint32_t res, uint8_t size);
uint8_t check_carry(uint32_t a, uint32_t b, uint32_t res, uint8_t size, bool substraction);
uint32_t truncate_val(uint32_t data, uint8_t size);
void set_flags_add(uint32_t src, uint32_t dst, uint8_t size, CPU* cpu);
void set_flags_sub(uint32_t src, uint32_t dst, uint8_t size, CPU* cpu);
bool check_condition(uint8_t condition, CCR ccr);