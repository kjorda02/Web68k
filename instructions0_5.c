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
    INS31233 ins;
    memcpy(&ins, &i, 2);

    if (ins.f1 == 0b100 || ins.f2) {
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
    INS31233 ins;
    memcpy(&ins, &i, 2);
    INS4233 ins2; // Only used to pass as argument to instruction implementation function
    memcpy(&ins2, &i, 2);

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

// TODO: Consider merging ori, andi & eori into one function
void ori(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);

    // TODO: PERMISSIONS
     // If operand in fields 4 and 5 indicates immediate, then the operation is done on the SR
    if (ins.f4 == 0b111 && ins.f5 == 0b100) { // (destination operand cannot be immedate)
        uint16_t sr = *((uint16_t*) &cpu->sr);
        sr |= srcOp.value; // It will be done on CCR or entire SR depending on the size of immediate operand
        cpu->sr = *((SR*) &sr);
    }
    else {
        operand dstOp = read_operand(size, ins.f4, ins.f5, false);
        dstOp.value |= srcOp.value;
        write_operand(dstOp, size);
        cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
        cpu->sr.ccr.zero = (truncate_val(dstOp.value, size) == 0);
        cpu->sr.ccr.overflow = 0;
        cpu->sr.ccr.carry = 0;
    }
}

void andi(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);

    // TODO: PERMISSIONS
    // If operand in fields 4 and 5 indicates immediate, then the operation is done on the SR
    if (ins.f4 == 0b111 && ins.f5 == 0b100) { // (destination operand cannot be immedate)
        uint16_t sr = *((uint16_t*) &cpu->sr);
        sr &= srcOp.value; // It will be done on CCR or entire SR depending on the size of immediate operand
        cpu->sr = *((SR*) &sr);
    }
    else {
        operand dstOp = read_operand(size, ins.f4, ins.f5, false);
        dstOp.value &= srcOp.value;
        write_operand(dstOp, size);
        cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
        cpu->sr.ccr.zero = (truncate_val(dstOp.value, size) == 0);
        cpu->sr.ccr.overflow = 0;
        cpu->sr.ccr.carry = 0;
    }
}

void subi(INS31233 ins, CPU* cpu) {
    printf("(SUBI)\n");
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    set_flags_sub(srcOp.value, dstOp.value, size, cpu);

    dstOp.value -= srcOp.value;
    write_operand(dstOp, size);
}

void addi(INS31233 ins, CPU* cpu) {
    printf("(ADDI)\n");
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    set_flags_add(srcOp.value, dstOp.value, size, cpu);

    dstOp.value += srcOp.value;
    write_operand(dstOp, size);
}

void eori(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);
    
    // TODO: PERMISSIONS
    // If operand in fields 4 and 5 indicates immediate, then the operation is done on the SR
    if (ins.f4 == 0b111 && ins.f5 == 0b100) { // (destination operand cannot be immedate)
        uint16_t sr;
        memcpy(&sr, &cpu->sr, 2);
        sr ^= srcOp.value; // It will be done on CCR or entire SR depending on the size of immediate operand
        memcpy(&cpu->sr, &sr, 2);
    }
    else {
        operand dstOp = read_operand(size, ins.f4, ins.f5, false);
        dstOp.value ^= srcOp.value;
        write_operand(dstOp, size);
        cpu->sr.ccr.negative = ( (int32_t) truncate_val(dstOp.value, size)) < 0;
        cpu->sr.ccr.zero = (truncate_val(dstOp.value, size) == 0);
        cpu->sr.ccr.overflow = 0;
        cpu->sr.ccr.carry = 0;
    }
}

void cmpi(INS31233 ins, CPU* cpu) {
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b111, 0b100, false);
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    int32_t res = truncate_val(dstOp.value - srcOp.value, size);
    cpu->sr.ccr.negative = res < 0;
    cpu->sr.ccr.zero = res == 0;
    cpu->sr.ccr.overflow = check_overflow(srcOp.value, dstOp.value, res, size);
    cpu->sr.ccr.carry = check_carry(srcOp.value, dstOp.value, res, size, true);
}

void bop(INS31233 ins, CPU* cpu) {
    printf("(BIT OPERATION)\n");

    operand numOp;
    if (ins.f2) // Bit number is stored in data register
        numOp = read_operand(BYTE, 0b000, ins.f1, false);
    else // Bit number is stored as immediate operand
        numOp = read_operand(BYTE, 0b111, 0b100, false);
    
    uint8_t num = numOp.value & 0x1F; // Modulo 32;
    uint8_t size = LONG;
    operand dstOp = read_operand(BYTE, ins.f4, ins.f5, true);
    if (dstOp.mem_access) {
        num &= 0x7; // Modulo 8
        size = BYTE;
    }
    dstOp = read_operand(size, ins.f4, ins.f5, false);
    
    cpu->sr.ccr.zero = !(dstOp.value & (1<<num)); // Set Z condition code accordingly
    switch (ins.f3) {
        case 0b00: // BTST
            return;
        case 0b01: // BCHG
            dstOp.value ^= (1<<num); // XOR operation to flip bit
            break;
        case 0b10: // BCLR
            dstOp.value &= ~(1<<num); // Negate mask to clear bit
            break;
        case 0b11: // BSET
            dstOp.value |= 1<<num;
            break;
    }
    write_operand(dstOp, size);
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 0101 ======================

void addq(INS31233 ins, CPU* cpu) {
    printf("(ADDQ)\n");
    uint8_t size = ins.f3;
    if (ins.f4 == 0b001) // Size is discarded with address registers, whole register always used
        size = LONG;
    
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    // Condition codes not altered if destination is address register
    if (dstOp.mem_access || dstOp.dataReg)
        set_flags_add(ins.f1, dstOp.value, size, cpu);

    dstOp.value += ins.f1;
    write_operand(dstOp, size);
}

void subq(INS31233 ins, CPU* cpu) {
    printf("(SUBQ)\n");
    uint8_t size = ins.f3;
    if (ins.f4 == 0b001)
        size = LONG;
    
    operand dstOp = read_operand(size, ins.f4, ins.f5, false);

    if (dstOp.mem_access || dstOp.dataReg)
        set_flags_sub(ins.f1, dstOp.value, size, cpu);

    dstOp.value -= ins.f1;
    write_operand(dstOp, size);
}

void Scc(INS4233 ins, CPU* cpu) {
    printf("(Scc)\n");
    operand dstOp = read_operand(BYTE, ins.f3, ins.f4, true); // Find effective address
    if (check_condition(ins.f1, cpu->sr.ccr))
        dstOp.value = 0xFF;
    else
        dstOp.value = 0;

    write_operand(dstOp, BYTE);
}
void DBcc(INS4233 ins, CPU* cpu) {
    printf("(DBcc)\n");
    if (check_condition(ins.f1, cpu->sr.ccr)) {
        cpu->pc += 2; // Skip displacement stored after instruction
        return;
    }

    operand op1 = read_operand(WORD, 0b000, ins.f4, false); // Read data register
    op1.value = ((int16_t) op1.value) - 1;
    write_operand(op1, WORD);
    printf("value: %d\n", ((int16_t) op1.value));
    if (((int16_t) op1.value) == -1) { // If the data register's new value is -1, end the loop'
        cpu->pc += 2; // Skip displacement stored after instruction
        return;
    }

    uint32_t pcval = cpu->pc; // IMPORTANT: SAVE VALUE OF PC BEFORE READING DISPLACEMENT VALUE
    operand op2 = read_operand(WORD, 0b111, 0b000, true); // Read displacement (absolute short address)
    cpu->pc = pcval + ((int16_t) op2.address);
}