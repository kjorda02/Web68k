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
    INS IR = {0, 0, 0, 0}; // Instruction register

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
    INS i = *((INS*) &cpu.ram[cpu.pc]);
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


// TODO: To CCR
void decode(INS ins) {
    static void (*execute[16])(INS, CPU) = {
        NULL, //ADDI, ANDI, BCHG, BCLR, BSET, BTST, CMPI, EORI, MOVEP, ORI, ORI TO CCR, ORI TO SR, SUBI
        &move,
        &move,
        &move,
        NULL, // CHK, CLR, EXT, ILLEGAL, JMP, JSR, LEA, LINK, MOVE TO CCR, MOVE FROM SR, MOVE TO SR, MOVE USP, MOVEM, NEG, NEGX, NOP, NOT, PEA, RESET, RTE, RTR, RTS, STOP, SWAP, TAS, TRAP, TRAPV, TST, UNLK
        NULL, // ADDQ, DBcc, Scc, SUBQ
        NULL,// BCC, BRA, BSR,
        NULL,// MOVEQ, ROL, ROR, ROXL, ROXR
        NULL,// DIVS, DIVU, OR, SBCD,
        NULL, // SUB, SUBA, SUBX,
        NULL,
        NULL, // CMP, CMPA, CMPM, EOR
        NULL, // ABCD, AND, EXG, MULS, MULU, NBCD
        NULL, // TIPO ALU (ADD, ADDA, ADDX, )
        NULL, // ASL, ASR, LSL, LSR
        NULL,
    };

    execute[ins.opcode](ins, cpu);  // Calls the function corresponding to the opcode
}


void decode_op0(INS i, CPU cpu) {
    INS31233 ins = *(INS31233*) &i;

    if (ins.f2) {  // Bit 7 is set

    }
    else {
        switch(ins.f1) {
            case 0b000:  // ORI to CCR
                break;
            case 0b001:
                break;
            case 0b010:
                break;
            case 0b011:
                break;
            case 0b100:
                break;
            case 0b101:
                break;
            case 0b110:
                break;
        }
    }
}

void decode_op4(INS ins, CPU cpu) {

}