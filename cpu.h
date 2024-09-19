#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct CPU {
    uint32_t d[8]; // D0-D7
    uint32_t a[8]; // A0-A7. (A7=Stack pointer)
    uint32_t pc; // Program counter
    uint32_t sr; // Status register

    uint8_t ram[16777216]; // Memory (2*^24 bytes)
} CPU;

extern CPU cpu;

void initCpu();
