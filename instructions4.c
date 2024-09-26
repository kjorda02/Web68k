#include "instructions.h"


/* --- DECODE_OP4 -----------------------------------------------------------------------------------
 * Decode instruction with opcode 0100. Calls the appropiate funnction to run the instruction
 * Check http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf if you want to understand anything
*/
void decode_op4(INS i, CPU cpu) {
    // We break the instruction bits into 5 fields:
    //   XXXX   XXXX  XX   XXX  XXX
    // (opcode) (f1) (f2) (f3) (f4)
    INS4233 ins = *(INS4233*) &i;

    INS3333 ins3 = *(INS3333*) &i; // For LEA and CHK instructions

    // We look at the next 4 bits after the opcode
    switch(i.opcode) { // 0100 XXXX XX XXX XXX
        case 0b0000:
            if (ins.f2 == 0b11)
                move_from_sr(ins, cpu);
            else
                negx(ins, cpu);
            break;
        case 0b0010:
                clr(ins, cpu);
            break;
        case 0b0100:
            if (ins.f2 == 0b11)
                move_to_ccr(ins, cpu);
            else
                neg(ins, cpu);
            break;
        case 0b0110:
            if (ins.f2 == 0b11)
                move_to_sr(ins, cpu);
            else
;               NOT(ins, cpu);
            break;
        case 0b1000: // 0100 1000 XX XXX XXX
                if (ins.f2 == 0b00) {
                    nbcd(ins, cpu);
                }
                else if (ins.f2 == 0b01) { // 0100 1000 01 XXX XXX
                    if (ins.f3 == 0b000)
                        swap(ins, cpu);
                    else
                        pea(ins, cpu);
                }
                else { // 0100 1000 [10|11] XXX XXX
                    if (ins.f3 == 0b000)
                        ext(ins, cpu);
                    else
 ;                      movem(ins, cpu);
                }
            break;
        case 0b1010: // 0100 1010 XX XXX XXX
            if (ins.f2 == 0b11) { // 0100 1010 11 XXX XXX
                if (ins.f3 == 0b111 && ins.f4 == 0b100)
                    illegal(cpu);
                else
;                   tas(ins, cpu);
            }
            else {
                tst(ins, cpu);
            }
            break;
        case 0b1100:
                movem(ins, cpu);
            break;
        case 0b1110:
            if (ins.f2 == 0b01) { // 0100 1110 01 XXX XXX
                if (!(ins.f3 & 0b110)) { // 0100 1110 01 00 XXXX
                    INS84 ins2 = *(INS84*) &i;
                    trap(ins2, cpu);
                }
                else { // 0100 1110 01 [^00]X XXX
                    switch(ins.f3) {
                        case 0b010:
                                link(ins, cpu);
                            break;
                        case 0b011:
                                unlk(ins, cpu);
                            break;
                        case 0b100:
                        case 0b101:
                            move_usp(ins, cpu);
                            break;
                        case 0b110: // 0100 1110 01 110 XXX
                            switch(ins.f4) {
                                case 0b000:
                                        reset(cpu);
                                    break;
                                case 0b001:
                                        nop(cpu);
                                    break;
                                case 0b010:
                                        stop(cpu);
                                    break;
                                case 0b011:
                                        rte(cpu);
                                    break;
                                case 0b101:
                                        rts(cpu);
                                    break;
                                case 0b110:
                                        trapv(cpu);
                                    break;
                                case 0b111:
                                        rtr(cpu);
                                    break;
                            }
                            break;
                    }
                }
            }
            else { // 0100 1110 [10|11] XXX XXX
                if (ins.f2 == 0b10)
                    jsr(ins, cpu);
                else if (ins.f2 == 0b11)
                    jmp(ins, cpu);
            }
            break;
        default: // 0100 XXX1 1X XXX XXX
            if (ins.f2 == 0b11)
                lea(ins3, cpu);
            else if (ins.f2 == 0b10)
                chk(ins3,  cpu);
            break;
    }
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 0100 ======================

void move_from_sr(INS4233 ins, CPU cpu){

}
void move_to_ccr(INS4233 ins, CPU cpu) {

}
void move_to_sr(INS4233 ins, CPU cpu) {

}
void negx(INS4233 ins, CPU cpu) {

}
void clr(INS4233 ins, CPU cpu) {

}
void neg(INS4233 ins, CPU cpu) {

}
void NOT(INS4233 ins, CPU cpu){

}
void ext(INS4233 ins, CPU cpu) {

}
void nbcd(INS4233 ins, CPU cpu) {

}
void swap(INS4233 ins, CPU cpu) {

}
void pea(INS4233 ins, CPU cpu) {

}
void illegal(CPU cpu) {

}
void tas(INS4233 ins, CPU cpu) {

}
void tst(INS4233 ins, CPU cpu) {

}
void trap(INS84 ins, CPU cpu) {

}
void link(INS4233 ins, CPU cpu) {

}
void unlk(INS4233 ins, CPU cpu) {

}
void move_usp(INS4233 ins, CPU cpu) {

}
void reset(CPU cpu) {

}
void nop(CPU cpu) {

}
void stop(CPU cpu) {

}
void rte(CPU cpu) {

}
void rts(CPU cpu) {

}
void trapv(CPU cpu) {

}
void rtr(CPU cpu) {

}
void jsr(INS4233 ins, CPU cpu) {

}
void jmp(INS4233 ins, CPU cpu) {

}
void movem(INS4233 ins, CPU cpu) {

}
void lea(INS3333 ins, CPU cpu) {

}
void chk(INS3333 ins, CPU cpu) {

}