#include "instructions.h"

static CPU cpu;

CPU* initCpu() {
    for (int i = 0; i < 8; i++) {
        cpu.d[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        cpu.a[i] = 0;
    }
    cpu.pc = cpu.sr = cpu.cycles = 0;

    memset(cpu.ram, 0, sizeof(cpu.ram));

    return &cpu;
}

/* --- START_PROGRAM -----------------------------------------------------------------------------------
 * RUNS THE MAIN FETCH-DECODE-EXECUTE LOOP, CALLING HELPER METHODS TO ACTUALLY PROCESS THE INSTRUCTIONS
*/
int run_program(uint32_t entryPoint) {
    cpu.pc = entryPoint;
    INS IR = {0, 0}; // Instruction register

    while (IR.opcode != 0b1111) {
        IR = fetch();
        decode(IR); // Decode calls execute
    }

    return 0;
}


/* --- FETCH -----------------------------------------------------------------------------------
 * Obtains the instruction pointed to by the PC and increments it by 2 (2 bytes per instruction)
*/
INS fetch() {
    uint16_t word = fetch_word(cpu.pc);
    printf("DECODING WORD: %x\n", word);
    INS i = *((INS*) &word);
    cpu.pc += 2;

    return i;
}
/* --- FETCH_WORD, FETCH_LONG -------------------------------------------------------------------
 * Reads the memory array, returning the data in big-endian format.
*/
uint16_t fetch_word(uint32_t pos) {
    // Reverse the order so that it's stored in big endian format once we cast to 16-bit data type
    uint8_t word[2] = {cpu.ram[pos+1], cpu.ram[pos]};

    return *((uint16_t*) &word);  // Cast to 16 bit uninsigned integer
}

uint16_t fetch_long(uint32_t pos) {
    return 0;
}


void decode(INS ins) {
    static void (*execute[16])(INS, CPU*) = {
        &decode_op0,
        &move,
        &move,
        &move,
        &decode_op4,
        &decode_op5,
        &Bcc,
        &moveq,
        &decode_op8,
        &decode_op9,
        NULL, // not used
        &decode_op11,
        &decode_op12,
        &decode_op13,
        &decode_op14,
        NULL, // not used
    };

    if (execute[ins.opcode] != NULL){
        execute[ins.opcode](ins, &cpu);  // Calls the function corresponding to the opcode
    }
}
// C00
// 0000 1100 0000 0000