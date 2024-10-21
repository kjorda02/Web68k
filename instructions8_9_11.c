#include "instructions.h"
#include "util.h"

/* --- DECODE_OP8, DECODE_OP9, DECODE_OP11  -------------------------------------------------------
 * Decode instructions with opcode 1000, 1001 and 1011, respectively. Calls the appropiate funnction
 * to run the instruction. Check http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf for details
*/
void decode_op8(INS i, CPU* cpu) {
    // We break the instruction bits into 6 fields:
    //   XXXX   XXX    X   XX  XXX  XXX
    // (opcode) (f1) (f2) (f3) (f4) (f5)
    INS31233 ins = *(INS31233*) &i;

    if (ins.f3 == 0b11) { // 1000 XXX X 11 XXX XXX
        if (ins.f2)
            divs(ins, cpu);
        else
            divu(ins, cpu);
    }
    else { // 1000 XXX X [^11] XXX XXX
        if (ins.f2 && (ins.f4 & 0b110) == 0)  // 1000 XXX 1 [^11] 00X XXX
            sbcd(ins, cpu);
        else
            OR(ins, cpu); // 1000 XXX X [^11] [^00]X XXX

            /* We are able to tell apart sbcd and OR because in the OR instruction, when the f2 bit is set,
             * that indicates that we are writing to the effective adress indicated by fields 4 and 5. It happens
             * that all adressing modes starting with 00 are not available in the OR instruction as destination operand.
             * (when the destination operand is a data register, f2 is set to 0 and f1 is used to indicate Dn)
            */
    }
}

void decode_op9(INS i, CPU* cpu) {
    INS31233 ins = *(INS31233*) &i;

    if (ins.f3 == 0b11) {
        suba(ins, cpu);
    }
    else { // 1001 XXX X [^11] XXX XXX
        // Same trick is used to tell apart SUBX and SUBA as with OR and SBCD
        if (ins.f2 && (ins.f4 & 0b110) == 0)  // 1001 XXX 1 [^11] 00X XXX
            subx(ins, cpu);
        else
            sub(ins, cpu);
    }

}

void decode_op11(INS i, CPU* cpu) {
    INS31233 ins = *(INS31233*) &i;

    if (ins.f3 == 0b11) {
        cmpa(ins, cpu);
    }
    else { // 1011 XXX X [^11] XXX XXX
        if (ins.f2) { // 1011 XXX 1 [^11] XXX XXX
            if (ins.f4 == 0b001) // M=001 (A_n adressing mode)
                cmpm(ins, cpu);
            else
                eor(ins, cpu); // EOR does not have adress register adressing mode, so no collision
        }
        else {
            cmp(ins, cpu);
        }
    }
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1000 ======================
void divu(INS31233 ins, CPU* cpu) {

}
void divs(INS31233 ins, CPU* cpu) {

}
void sbcd(INS31233 ins, CPU* cpu) {

}
void OR(INS31233 ins, CPU* cpu) {
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
    dstOp.value = srcOp.value | dstOp.value;
    write_operand(dstOp, size);

    cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
    cpu->sr.ccr.zero = (dstOp.value == 0);
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1011 ======================
void sub(INS31233 ins, CPU* cpu) {
    printf("(SUB)\n");
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

    set_flags_sub(srcOp.value, dstOp.value, size, cpu);

    dstOp.value = dstOp.value - srcOp.value;
    write_operand(dstOp, size);
}
void subx(INS31233 ins, CPU* cpu) {

}

// Does not alter condition codes
void suba(INS31233 ins, CPU* cpu) {
    printf("(SUBA)\n");
    uint8_t size = WORD;
    if (ins.f2)
        size = LONG;

    operand srcOp = read_operand(size, ins.f4, ins.f5, false);
    operand dstOp = read_operand(LONG, 0b001, ins.f1, false); // An is always long when used as destination operand!

    dstOp.value -= srcOp.value;
    write_operand(dstOp, size);
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1011 ======================
void eor(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;

    // <ea> + Dn -> <ea>
    operand srcOp = read_operand(size, 0b000, ins.f1, false); // Read Dn
    operand dstOp = read_operand(size, ins.f4, ins.f5, false); // Read <ea>
    
    dstOp.value ^= srcOp.value;
    write_operand(dstOp, size);

    cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
    cpu->sr.ccr.zero = (truncate_val(dstOp.value, size) == 0); // Source operands could be negative!
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}
void cmpm(INS31233 ins, CPU* cpu) {

}
void cmp(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(size, 0b000, ins.f1, false); // Read Dn

    int32_t res = truncate_val(dstOp.value - srcOp.value, size);
    cpu->sr.ccr.negative = res < 0;
    cpu->sr.ccr.zero = res == 0;
    cpu->sr.ccr.overflow = check_overflow(srcOp.value, dstOp.value, res, size);
    cpu->sr.ccr.carry = check_carry(srcOp.value, dstOp.value, res, size, true);
}

void cmpa(INS31233 ins, CPU* cpu) {
    uint8_t size = WORD;
    if (ins.f2)
        size = LONG;

    operand srcOp = read_operand(size, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(size, 0b001, ins.f1, false); // Read An

    int32_t res = truncate_val(dstOp.value - srcOp.value, size);
    cpu->sr.ccr.negative = res < 0;
    cpu->sr.ccr.zero = res == 0;
    cpu->sr.ccr.overflow = check_overflow(srcOp.value, dstOp.value, res, size);
    cpu->sr.ccr.carry = check_carry(srcOp.value, dstOp.value, res, size, true);
}
