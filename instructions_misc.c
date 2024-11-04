#include "instructions.h"
#include "util.h"

void move(INS i, CPU* cpu) {
    printf("(MOVE)\n");
    INS3333 ins = *((INS3333*) &i);
    uint8_t size = ins.opcode & 0b11; // lower 2 bits of the opcode indicate size
    if (size == 0b01)
        size = BYTE;
    else if (size == 0b11) // Translate to standard size codification
        size = WORD;

    operand srcOp = read_operand(size, ins.f3, ins.f4, false);
    operand dstOp = read_operand(size, ins.f2, ins.f1, true);
    dstOp.value = srcOp.value;
    write_operand(dstOp, size);

    CCR* flags = &cpu->sr.ccr;
    flags->overflow = 0;
    flags->carry = 0;
    flags->negative = ((int32_t) dstOp.value) < 0;
    flags->zero = (dstOp.value == 0);
}

void moveq(INS i, CPU* cpu) {
    printf("(MOVEQ)\n");
    INS318 ins = *(INS318*) &i;
    operand dstOp = read_operand(LONG, 0b000, ins.f1, true);
    dstOp.value = (int8_t) ins.f3; // Casting to signed int for sign extension

    write_operand(dstOp, LONG);

    cpu->sr.ccr.negative = ((int32_t) dstOp.value) < 0;
    cpu->sr.ccr.zero = (dstOp.value == 0);
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}

void Bcc(INS i, CPU* cpu) {
    INS48 ins = *(INS48*) &i;
    if (ins.cond == 0b0001) {
        bsr(ins, cpu);
        return;
    }
    printf("(Bcc)\n");
    if (!check_condition(ins.cond, cpu->sr.ccr)) {
        if (ins.disp == 0)
            cpu->pc += 2; // Skip displacement stored after instruction
        return;
    }

    if (ins.disp == 0) {
        uint32_t pcval = cpu->pc; // IMPORTANT: SAVE VALUE OF PC BEFORE READING DISPLACEMENT VALUE
        operand srcOp = read_operand(WORD, 0b111, 0b100, false);
        cpu->pc = pcval + ((int16_t) srcOp.value);
    }
    else {
        cpu->pc += (int8_t) ins.disp;
    }
}

void bsr(INS48 ins, CPU* cpu) {
    printf("(BSR)\n");
    
    uint32_t jmpdir; // Calculate jump address
    if (ins.disp == 0) {
        uint32_t pcval = cpu->pc; // IMPORTANT: SAVE VALUE OF PC BEFORE READING DISPLACEMENT VALUE
        operand srcOp = read_operand(WORD, 0b111, 0b100, false);
        jmpdir = pcval + ((int16_t) srcOp.value);
    }
    else 
        jmpdir = cpu->pc + (int8_t) ins.disp;
    
    // Save return address
    cpu->a[7] -= 4; // SP <- [SP] - 4
    operand op = {cpu->pc, cpu->a[7], true, false, 0};
    write_operand(op, LONG); // [SP] <- [PC]
    
    cpu->pc = jmpdir; // PC <- subroutine dir
}