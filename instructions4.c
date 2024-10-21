#include "instructions.h"
#include "util.h"


/* --- DECODE_OP4 -----------------------------------------------------------------------------------
 * Decode instruction with opcode 0100. Calls the appropiate funnction to run the instruction
 * Check http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf for details
*/
void decode_op4(INS i, CPU* cpu) {
    // We break the instruction bits into 5 fields:
    //   XXXX   XXXX  XX   XXX  XXX
    // (opcode) (f1) (f2) (f3) (f4)
    INS4233 ins = *(INS4233*) &i;

    INS3333 ins3 = *(INS3333*) &i; // For LEA and CHK instructions

    // We look at the next 4 bits after the opcode
    switch(ins.f1) { // 0100 XXXX XX XXX XXX
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
                NOT(ins, cpu);
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
                        movem(ins, cpu);
                }
            break;
        case 0b1010: // 0100 1010 XX XXX XXX
            if (ins.f2 == 0b11) { // 0100 1010 11 XXX XXX
                if (ins.f3 == 0b111 && ins.f4 == 0b100)
                    illegal(cpu);
                else
                    tas(ins, cpu);
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

void move_from_sr(INS4233 ins, CPU* cpu){

}
void move_to_ccr(INS4233 ins, CPU* cpu) {

}
void move_to_sr(INS4233 ins, CPU* cpu) {

}
void negx(INS4233 ins, CPU* cpu) {

}
void clr(INS4233 ins, CPU* cpu) {
    printf("(CLR)\n");
    uint8_t size = ins.f2;
    operand dstOp = read_operand(size, ins.f3, ins.f4, true);
    dstOp.value = 0;
    write_operand(dstOp, size);

    cpu->sr.ccr.negative = 0;
    cpu->sr.ccr.zero = 1;
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}
void neg(INS4233 ins, CPU* cpu) {
    uint8_t size = ins.f2;
    operand dstOp = read_operand(size, ins.f3, ins.f4, false);
    set_flags_sub(dstOp.value, 0, size, cpu);
    dstOp.value = -dstOp.value;;
    write_operand(dstOp, size);
}
void NOT(INS4233 ins, CPU* cpu){
    uint8_t size = ins.f2;
    operand dstOp = read_operand(size, ins.f3, ins.f4, false);
    dstOp.value = ~dstOp.value;
    write_operand(dstOp, size);

    cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
    cpu->sr.ccr.zero = (truncate_val(dstOp.value, size) == 0);
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}
void ext(INS4233 ins, CPU* cpu) {

}
void nbcd(INS4233 ins, CPU* cpu) {

}
void swap(INS4233 ins, CPU* cpu) {

}
void pea(INS4233 ins, CPU* cpu) {

}
void illegal(CPU* cpu) {

}
void tas(INS4233 ins, CPU* cpu) {

}
void tst(INS4233 ins, CPU* cpu) {
    printf("(TST)\n");
    uint8_t size = ins.f2;
    operand srcOp = read_operand(size, ins.f3, ins.f4, false);
    set_flags_sub(0, srcOp.value, size, cpu);
}
void trap(INS84 ins, CPU* cpu) {
    printf("(TRAP)\n");
}
void link(INS4233 ins, CPU* cpu) {

}
void unlk(INS4233 ins, CPU* cpu) {

}
void move_usp(INS4233 ins, CPU* cpu) {

}
void reset(CPU* cpu) {

}
void nop(CPU* cpu) {

}
void stop(CPU* cpu) {

}
void rte(CPU* cpu) {

}
void rts(CPU* cpu) {
    printf("(RTS)\n");
    operand srcOp = read_operand(LONG, 0b010, 7, false); // Read (A7)
    cpu->pc = srcOp.value; // PC <- [[SP]]
    cpu->a[7] += 4; // SP <- [SP] + 4
}
void trapv(CPU* cpu) {

}
void rtr(CPU* cpu) {
    

}
void jsr(INS4233 ins, CPU* cpu) {
    printf("(JSR)\n");
    cpu->a[7] -= 4; // SP <- [SP] - 4
    operand op = {cpu->pc, cpu->a[7], true, false, 0};
    write_operand(op, LONG); // [SP] <- [PC]
    
    operand srcOp = read_operand(LONG, ins.f3, ins.f4, true);
    cpu->pc = srcOp.address; // PC <- subroutine dir
}

void jmp(INS4233 ins, CPU* cpu) {

}
void movem(INS4233 ins, CPU* cpu) {

}
void lea(INS3333 ins, CPU* cpu) {
    printf("(LEA)\n");
    operand srcOp = read_operand(LONG, ins.f3, ins.f4, true);
    operand dstOp = read_operand(LONG, 0b001, ins.f1,true);

    dstOp.value = srcOp.address;
    write_operand(dstOp, LONG);
}
void chk(INS3333 ins, CPU* cpu) {

}