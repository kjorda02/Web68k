#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdint.h>
#include<string.h>
#include <math.h>

#define ALLOW_ALL 0x1FFF
#define ALLOW_MEM_ALT 0x03FC  // Allow only memory alterable adressing modes
#define ALLOW_MEM_ALT_DN 0x03FD // Also allow data register direct
#define ALLOW_NO_AN 0x1FFFD

#define BYTE 0b00
#define WORD 0b01
#define LONG 0b10

/* The following structs are used to ease the manipulation of bit fields, avoiding
 * unnecessary bit operations. All fields are in reverse order, since struct fields
 * are stored from last to first in memory, so it cancels out. Additionally, for structs
 * larger than one byte, it is necessary to reverse the order of the bytes before copying
 * them into the struct's memory. ALL CODE ASSUMES IT WILL RUN ON A LITTLE ENDIAN MACHINE.
*/

// Condition code register
typedef struct CCR {
    uint8_t carry : 1;
    uint8_t overflow : 1;
    uint8_t zero : 1;
    uint8_t negative : 1;
    uint8_t extend : 1;
    uint8_t garbage : 3;
} CCR;

// Status register
typedef struct SR {
    CCR ccr;
    uint8_t interrupt_mask : 3;
    uint8_t garbage : 2;
    uint8_t supervisor_state : 1;
    uint8_t garbage2 : 1;
    uint8_t trace_mode : 1;
} SR;

typedef struct CPU {
    uint32_t d[8]; // D0-D7
    uint32_t a[8]; // A0-A7. (A7=Stack pointer)
    uint32_t pc; // Program counter
    SR sr; // Status register
    unsigned long cycles;

    uint8_t ram[16777216]; // Memory (2*^24 bytes)
} CPU;

typedef struct INS {
    uint16_t f1 : 12;
    uint16_t opcode : 4;
} INS;

typedef struct BEW { // Brief extension word (Indirect and relative with index adressing modes)
    uint16_t displacement : 8; // 8-bit displacement
    uint16_t garbage : 3;
    uint16_t lon : 1; // 1=long, 0=word
    uint16_t n : 3;
    uint16_t mode : 1; // 0=Dn, 1=An
} BEW;

CPU* initCpu();
int run_program(uint32_t entryPoint);
void decode(INS ins);
INS fetch();
uint32_t read_mem(uint32_t pos, uint8_t size);
void write_mem(uint32_t pos, uint8_t size, uint32_t data);
uint32_t fetch_data(uint8_t size);
void write_Dn(uint32_t data, uint8_t n, uint8_t size);
void write_An(uint32_t data, uint8_t n, uint8_t size);
uint32_t read_Dn(uint8_t n, uint8_t size);
uint32_t read_An(uint8_t n, uint8_t size);
int access_operand(uint32_t* buf, uint8_t size, uint16_t allowed_addr_modes, uint8_t M, uint8_t Xn, bool write);

// The following structs break the 16-bit instructions into different fields.
// For instance, INS31233 would break the instructions into the following fields:
//   XXXX   XXX    X   XX  XXX  XXX
// (opcode) (f1) (f2) (f3) (f4) (f5)
typedef struct INS31233 {
    uint16_t f5 : 3;
    uint16_t f4 : 3;
    uint16_t f3 : 2;
    uint16_t f2 : 1;
    uint16_t f1 : 3;
    uint16_t opcode : 4;
} INS31233;

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

typedef struct INS84 {
    uint16_t f2 : 4;
    uint16_t f1 : 8;
    uint16_t opcode : 4;
} INS84;

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