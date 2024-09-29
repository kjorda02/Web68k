#include "instructions.h"

void move(INS i, CPU* cpu) {
    INS3333 ins = *((INS3333*) &i);
    uint32_t data;
    uint8_t size = ins.opcode & 0b11; // lower 2 bits of the opcode indicate size
    if (size == 0b01)
        size = 0b00;
    else if (size == 0b11) // Translate to standard size codification
        size = 0b01;

    operand srcOp = read_operand(size, ins.f3, ins.f4, false);
    operand dstOp = read_operand(size, ins.f2, ins.f1, true);
    dstOp.value = srcOp.value;
    write_operand(dstOp, size);
    // TODO: Flags
}

void moveq(INS i, CPU* cpu) {
    printf("MOVEQ\n");
    INS318 ins = *(INS318*) &i;
}

void Bcc(INS i, CPU* cpu) {
    INS48 ins = *(INS48*) &i;
}