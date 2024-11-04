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
            mult(ins, cpu, true);
        else
            mult(ins, cpu, false);
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

    shiftop(ins, cpu, type);
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1100 ======================

void mult(INS31233 ins, CPU* cpu, bool sign) {
    printf("(MULU/MULS)\n");
    operand srcOp = read_operand(WORD, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(WORD, 0b000, ins.f1, false); // Read Dn
    srcOp.value &= 0xFFFF; // Truncate to word
    dstOp.value &= 0xFFFF;
    
    if (sign)
        dstOp.value = ( (int16_t) dstOp.value)*( (int16_t) srcOp.value);
    else
        dstOp.value *= srcOp.value;
    write_operand(dstOp, LONG);
    
    cpu->sr.ccr.negative = ( (int32_t) dstOp.value) < 0;
    cpu->sr.ccr.zero = dstOp.value == 0;
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
}

void abcd(INS31233 ins, CPU* cpu) {

}

void exg(INS31233 ins, CPU* cpu) {
    uint8_t mode = ins.f4;
    operand ry = read_operand(LONG, mode, ins.f5, false);
    if (ins.f3 == 0b10) // Different types (data and address)
        mode = 0b000;
    
    operand rx = read_operand(LONG, mode, ins.f1, false);
    
    uint32_t aux = rx.value;
    rx.value = ry.value;
    ry.value = aux;
    write_operand(rx, LONG);
    write_operand(ry, LONG);
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

// Arithmetic shift helper function
uint32_t ashift(CPU* cpu, uint32_t val, bool left, uint8_t count, uint8_t msb) {
    if (left) { // Left shift
        bool sign = val>>msb; // Save most significant bit state
        for ( ; count > 1; count--) {
            val *= 2;
            cpu->sr.ccr.overflow |= (val>>msb != sign); // If changed -> overflow
        }
        cpu->sr.ccr.extend = val>>msb; // Last bit shifted out
        cpu->sr.ccr.carry = val>>msb;
        val *= 2;
        cpu->sr.ccr.overflow |= (val>>msb != sign); // If changed -> overflow
    }
    else { // Right shift
        for ( ; count > 1; count--) {
            val /= 2;
        }
        cpu->sr.ccr.extend = val & 1; // Last bit shifted out
        cpu->sr.ccr.carry = val & 1;
        val /= 2;
    }
    cpu->sr.ccr.negative = val>>msb;
    cpu->sr.ccr.zero = (val & (0xFFFFFFFF>>(32-(msb+1)))) == 0; // Mask in case of left shift
    return val;
}

// Logical shift helper function
uint32_t lshift(CPU* cpu, uint32_t val, bool left, uint8_t count, uint8_t msb) {
    if (left) {
        val <<= (count-1);
        cpu->sr.ccr.extend = val>>msb; // Last bit shifted out
        cpu->sr.ccr.carry = val>>msb;
        val <<= 1;
    }
    else {
        val >>= (count-1);
        cpu->sr.ccr.extend = val & 1; // Last bit shifted out
        cpu->sr.ccr.carry = val & 1;
        val >>= 1;
    }
    cpu->sr.ccr.negative = val>>msb;
    cpu->sr.ccr.zero = (val & (0xFFFFFFFF>>(32-(msb+1)))) == 0;
    cpu->sr.ccr.overflow = 0;
    return val;
}

// Rotate helper function
uint32_t rotate(CPU* cpu, uint32_t val, bool left, uint8_t count, uint8_t msb) {
    if (left) {
        val = val<<count | val>>(msb+1-count);
        cpu->sr.ccr.carry = val & 1;
    }
    else {
        val = val>>count | val<<(msb+1-count);
        cpu->sr.ccr.carry = (val>>msb) & 1;
    }
    
    cpu->sr.ccr.negative = (val>>msb) & 1;
    cpu->sr.ccr.zero = (val & (0xFFFFFFFF>>(32-(msb+1)))) == 0;
    cpu->sr.ccr.overflow = 0;
    return val;
}

// Rotate with extend helper function
uint32_t rotatex(CPU* cpu, uint32_t val2, bool left, uint8_t count, uint8_t msb) {
    uint64_t val = val2; // We're just going to pretend the extend bit is immediately to the
                        // left of our data. So uint32_t is not enough for long word anymore
    
    if (left) {
        val = val<<count | val>>(msb+2-count); // We treat the calculations as if our data was 1 bit larger
    }
    else {
        val = val>>count | val<<(msb+2-count);
    }
    
    cpu->sr.ccr.carry = (val>>(msb+1)) & 1;
    cpu->sr.ccr.extend = (val>>(msb+1)) & 1;
    cpu->sr.ccr.negative = (val>>msb) & 1;
    cpu->sr.ccr.zero = (val & (0xFFFFFFFF>>(32-(msb+1)))) == 0;
    cpu->sr.ccr.overflow = 0;
    
    return val;
}

// === IMPLEMENTATION FOR INSTRUCTIONS WITH OPCODE: 1110 ======================
void shiftop(INS31233 ins, CPU* cpu, uint8_t type) {
    printf("(SHIFT/ROTATE OPERATION)\n");
    operand dstOp;
    uint8_t count;
    uint8_t size = WORD;
    uint8_t msb = 7; // Position of most significant bit
    if (ins.f3 == 0b11) { // Specifies memory shift, only shift by one and size is word
        dstOp = read_operand(WORD, ins.f4, ins.f5, false);
        count = 1;
    }
    else { // Specifies data register shift, size can be byte, word or long. Count can be 0-63
        dstOp = read_operand(ins.f3, 0b000, ins.f5, false);
        size = ins.f3;
        
        count = ins.f1; // If bit 5 is 0, bits 9-11 specify the shift count
        if (ins.f4 & 0b100) // Otherwise, they specify the data register that contains the count
            count = cpu->d[ins.f1] & 0x3F; // Modulo 64
        else if (count == 0)
            count = 8;
    }
    
    msb = size_to_bytes(size)*8 -1; // Position of the most significant bit
    dstOp.value &= 0xFFFFFFFF>>(32-(msb+1)); // Clear higher bits in case of right shift!
    
    switch(type) {
        case 0b00:
            dstOp.value = ashift(cpu, dstOp.value, ins.f2, count, msb);
            break;
        case 0b01:
            dstOp.value = lshift(cpu, dstOp.value, ins.f2, count, msb);
            break;
        case 0b10:
            dstOp.value = rotatex(cpu, dstOp.value, ins.f2, count, msb);
            break;
        case 0b11:
            dstOp.value = rotate(cpu, dstOp.value, ins.f2, count, msb);
            break;
    }
    
    if (count == 0 && type != 0b10)
        cpu->sr.ccr.carry = 0;
    
    write_operand(dstOp, size);
}