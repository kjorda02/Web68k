#include "instructions.h"

void decode_op8(INS i, CPU cpu) {
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

void decode_op9(INS i, CPU cpu) {
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

void decode_op11(INS i, CPU cpu) {
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
void divu(INS31233 ins, CPU cpu) {

}
void divs(INS31233 ins, CPU cpu) {

}
void sbcd(INS31233 ins, CPU cpu) {

}
void OR(INS31233 ins, CPU cpu) {

}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1011 ======================
void sub(INS31233 ins, CPU cpu) {

}
void subx(INS31233 ins, CPU cpu) {

}
void suba(INS31233 ins, CPU cpu) {

}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1011 ======================
void eor(INS31233 ins, CPU cpu) {

}
void cmpm(INS31233 ins, CPU cpu) {

}
void cmp(INS31233 ins, CPU cpu) {

}
void cmpa(INS31233 ins, CPU cpu) {

}
