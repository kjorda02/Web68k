#include "instructions.h"
#include "util.h"

/* --- DECODE_OP12, DECODE_OP13, DECODE_OP14  -------------------------------------------------------
 * Decode instructions with opcode 1100, 1101 and 1110, respectively. Calls the appropiate funnction
 * to run the instruction. Check http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf for details
*/
void decode_op12(INS i, CPU* cpu) {
    // We break the instruction bits into 6 fields:
    //   XXXX   XXX    X   XX  XXX  XXX
    // (opcode) (f1) (f2) (f3) (f4) (f5)
    INS31233 ins = *(INS31233*) &i;

    if (ins.f3 == 0b11) { // 1100 XXX X 11 XXX XXX
        if (ins.f2)
            muls(ins, cpu);
        else
            mulu(ins, cpu);
    }
    else if (ins.f2 == 0) { // 1100 XXX 0 [^11] XXX XXX
        AND(ins, cpu);
    }
    else if ((ins.f4 & 0b110) == 0){ // 1100 XXX 1 [^11] 00X XXX
        if (ins.f3 == 0)
            abcd(ins, cpu);
        else
            exg(ins, cpu);
    }
    else { // 1100 XXX 1 [^11] [^00]X XXX
        AND(ins, cpu);
    }
}


void decode_op13(INS i, CPU* cpu) {
    INS31233 ins = *(INS31233*) &i;

    if (ins.f3 == 0b11) {
        adda(ins, cpu);
    }
    else if (ins.f2 == 1 && (ins.f4 & 0b110) == 0){ // 1101 XXX X [^11] 00X XXX
        addx(ins, cpu);
    }
    else {
        add(ins, cpu);
    }
}

void decode_op14(INS i, CPU* cpu) {
    INS31233 ins = *(INS31233*) &i;

    uint8_t type;;
    if (ins.f3 == 0b11) // The destination operand is in memory (only Word size and only 1 bit rotation)'
        type = ins.f1;
    else  // The destination operand is a data register.
        type = ins.f4 & 0b011;

    switch(type) {
        case 0b00:
            ASd(ins, cpu);
        case 0b01:
            LSd(ins, cpu);
        case 0b10:
            ROXd(ins, cpu);
        case 0b11:
            ROd(ins, cpu);
    }
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1100 ======================

void mulu(INS31233 ins, CPU* cpu) {

}
void muls(INS31233 ins, CPU* cpu) {

}
void abcd(INS31233 ins, CPU* cpu) {

}
void exg(INS31233 ins, CPU* cpu) {

}
void AND(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;

    // <ea> + Dn -> Dn
    operand srcOp = read_operand(size, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(size, 0b000, ins.f1, false); // Read Dn

    // Dn + <ea> -> <ea>
    if (ins.f2) {
        operand aux = srcOp;
        srcOp = dstOp;
        dstOp = aux;
    }
    dstOp.value = srcOp.value & dstOp.value;
    write_operand(dstOp, size);

    cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
    cpu->sr.ccr.zero = (truncate_val(dstOp.value, size) == 0);
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1101 ======================

void add(INS31233 ins, CPU* cpu) {
    printf("(ADD)\n");
    uint8_t size = ins.f3;

    // <ea> + Dn -> Dn
    operand srcOp = read_operand(size, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(size, 0b000, ins.f1, false); // Read Dn

    if (ins.f2) { // Dn + <ea> -> <ea>
        operand aux = srcOp;
        srcOp = dstOp;
        dstOp = aux;
    }
    set_flags_add(srcOp.value, dstOp.value, size, cpu);

    dstOp.value = srcOp.value + dstOp.value;
    write_operand(dstOp, size);
}
void addx(INS31233 ins, CPU* cpu) {

}

// Does not alter condition codes
void adda(INS31233 ins, CPU* cpu) {
    printf("(ADDA)\n");
    uint8_t size = WORD;
    if (ins.f2)
        size = LONG;

    operand srcOp = read_operand(size, ins.f4, ins.f5, false);
    operand dstOp = read_operand(LONG, 0b001, ins.f1, false); // An is always long when used as destination operand!

    dstOp.value += srcOp.value;
    write_operand(dstOp, size);
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1110 ======================
void ASd(INS31233 ins, CPU* cpu) {

}
void LSd(INS31233 ins, CPU* cpu) {

}
void ROXd(INS31233 ins, CPU* cpu) {

}
void ROd(INS31233 ins, CPU* cpu) {

}

