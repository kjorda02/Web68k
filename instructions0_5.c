#include "instructions.h"
#include "util.h"

/* --- DECODE_OP0, DECODE OP5 ---------------------------------------------------------------------------
 * Decode instructions with opcode 0000 and 0101, respectively.. Calls the appropiate funnction to run
 * the instruction. Check http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf for details
*/
void decode_op0(INS i, CPU* cpu) {
    // We break the instruction bits into 6 fields:
    //   XXXX   XXX    X   XX  XXX  XXX
    // (opcode) (f1) (f2) (f3) (f4) (f5)
    INS31233 ins = *(INS31233*) &i;

    if (ins.f2) {  // Bit 7 is set
        bop(ins, cpu);
    }
    else {
        switch(ins.f1) {
            case 0b000:
                ori(ins, cpu);
                break;
            case 0b001:
                andi(ins, cpu);
                break;
            case 0b010:
                subi(ins, cpu);
                break;
            case 0b011:
                addi(ins, cpu);
                break;
            case 0b100:
                //(i, cpu);
                break;
            case 0b101:
                eori(ins, cpu);
                break;
            case 0b110:
                cmpi(ins, cpu);
                break;
        }
    }
}

void decode_op5(INS i, CPU* cpu) {
    INS31233 ins = *(INS31233*) &i;
    INS4233 ins2 = *(INS4233*) &i; // Only used to pass as argument to instruction implementation function

    if (ins.f3 == 0b11) {
        if (ins.f4 == 0b001)
            DBcc(ins2, cpu);
        else
            Scc(ins2, cpu);
    }
    else { // 0101 X [^11] XXX XXX
        if (ins.f2)
            subq(ins, cpu);
        else
            addq(ins, cpu);
    }
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 0000 ======================

void ori(INS31233 Ins, CPU* cpu) {
    // TODO: If operand in fields 4 and 5 indicates immediate, then the operation is done on
    // the SR (destination operand cannot be immedate). Check field 3 as usual to determine if
    // operation is done only on CCR or on the whole SR

    // if (!(ins.f4 == 0b111 && ins.f5 == 0b100))
    //     ori(i, cpu);
    // else if (ins.f3 == 0b00)
    //     ori_to_ccr(i, cpu);
    // else if (ins.f3 == 0b01)
    //     ori_to_sr(i, cpu);
}

void andi(INS31233 ins, CPU* cpu) {
    // TODO: Check fields 4 and 5 as in ori
}

void subi(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);
    uint32_t op2 = dstOp.value;
    dstOp.value -= srcOp.value;
    write_operand(dstOp, size);

    cpu->sr.ccr.overflow = check_overflow(-ins.f1, op2, dstOp.value, size);
}

void addi(INS31233 ins, CPU* cpu) {

}

void eori(INS31233 ins, CPU* cpu) {
    // TODO: Check for possible SR or CCR operation as in ori
}

void cmpi(INS31233 ins, CPU* cpu) {

}

void bop(INS31233 Ins, CPU* cpu) {
    // TODO: Check field 2 to see if bit number is stored as immediate (fetch necessary)
    // Check field 3 to decode between BTST, BCHG, BCLR and BSET
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 0101 ======================

void addq(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    // Condition codes not altered if destination is address register
    if (dstOp.mem_access || dstOp.dataReg)
        set_flags_add(ins.f1, dstOp.value, size, cpu);

    dstOp.value += ins.f1;
    write_operand(dstOp, size);
}

void subq(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    if (dstOp.mem_access || dstOp.dataReg)
        set_flags_sub(ins.f1, dstOp.value, size, cpu);

    dstOp.value -= ins.f1;
    write_operand(dstOp, size);
}

void Scc(INS4233 ins, CPU* cpu) {

}
void DBcc(INS4233 ins, CPU* cpu) {

}