#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdint.h>
#include<string.h>

typedef struct CPU {
    uint32_t d[8]; // D0-D7
    uint32_t a[8]; // A0-A7. (A7=Stack pointer)
    uint32_t pc; // Program counter
    uint32_t sr; // Status register
    unsigned long cycles;

    uint8_t ram[16777216]; // Memory (2*^24 bytes)
} CPU;

typedef struct INS {
    uint16_t f1 : 12;
    uint16_t opcode : 4;
} INS;

typedef struct INS31233 {
    uint16_t f5 : 3;
    uint16_t f4 : 3;
    uint16_t f3 : 2;
    uint16_t f2 : 1;
    uint16_t f1 : 3;
    uint16_t opcode : 4;
} INS31233;

typedef struct INS311133 {
    uint16_t f6 : 3;
    uint16_t f5 : 3;
    uint16_t f4 : 1;
    uint16_t f3 : 1;
    uint16_t f2 : 1;
    uint16_t f1 : 3;
    uint16_t opcode : 4;
} INS311133;

// Used by MOVE
typedef struct INS3333 {
    uint16_t f4 : 3;
    uint16_t f3 : 3;
    uint16_t f2 : 3;
    uint16_t f1 : 3;
    uint16_t opcode : 4;
} INS3333;

typedef struct INS4233 {
    uint16_t f4 : 3;
    uint16_t f3 : 3;
    uint16_t f2 : 2;
    uint16_t f1 : 4;
    uint16_t opcode : 4;
} INS4233;

// Used by 1-operand ALU instructions
typedef struct INS633 {
    uint16_t f3 : 3;
    uint16_t f2 : 3;
    uint16_t f1 : 6;
    uint16_t opcode : 4;
} INS633;

typedef struct INS84 {
    uint16_t f2 : 4;
    uint16_t f1 : 8;
    uint16_t opcode : 4;
} INS84;

typedef struct INS813 {
    uint16_t f3 : 3;
    uint16_t f2 : 1;
    uint16_t f1 : 8;
    uint16_t opcode : 4;
} INS813;

// Used by branch instructions
typedef struct INS48 {
    uint16_t dir : 8;
    uint16_t cond : 4;
    uint16_t opcode : 4;
} INS48;

typedef struct INS318 {
    uint16_t f3 : 8;
    uint16_t f2 : 1;
    uint16_t f1 : 3;
    uint16_t opcode : 4;
} INS318;

// Used by 2-operand ALU instructions when the operands are in the form Dm, Dn or #k, b
typedef struct INS363 {
    uint16_t f3 : 3;
    uint16_t f2 : 6;
    uint16_t f1 : 3;
    uint16_t opcode : 4;
} INS363;

// TODO: Check endianness / order
typedef struct BYTEI {
    uint8_t garbage;
    uint8_t byte;
} BYTEI;

CPU* initCpu();
int run_program(uint32_t entryPoint);
void decode(INS ins);
INS fetch();
uint16_t fetch_word(uint32_t pos);
uint16_t fetch_long(uint32_t pos);