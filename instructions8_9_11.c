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
    INS31233 ins;
    memcpy(&ins, &i, 2);

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
    INS31233 ins;
    memcpy(&ins, &i, 2);

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
    INS31233 ins;
    memcpy(&ins, &i, 2);

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
    operand srcOp = read_operand(WORD, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(LONG, 0b000, ins.f1, false); // Read Dn
    srcOp.value &= 0xFFFF; // Truncate to word
    
    if ((dstOp.value>>16) >= srcOp.value) {
        cpu->sr.ccr.overflow = 1;
        return;
    }
    
    // TODO: CHECK DIVISION BY ZERO
    uint16_t quotient = dstOp.value/srcOp.value;
    uint16_t remainder = dstOp.value % srcOp.value;
    
    dstOp.value = quotient;
    dstOp.value |= remainder<<16;
    write_operand(dstOp, LONG);
    
    cpu->sr.ccr.carry = 0;
    cpu->sr.ccr.negative = ( (int16_t) quotient) < 0;
    cpu->sr.ccr.zero = (quotient == 0);
    cpu->sr.ccr.overflow = 0;
}

void divs(INS31233 ins, CPU* cpu) {
    operand srcOp = read_operand(WORD, ins.f4, ins.f5, false); // Read <ea>
    operand dstOp = read_operand(LONG, 0b000, ins.f1, false); // Read Dn
    int16_t divisor = srcOp.value & 0xFFFF; // Truncate to word
    int32_t dividend = dstOp.value;
    
    if ((dividend/divisor)>>16) {
        cpu->sr.ccr.overflow = 1;
        return;
    }
    
    // TODO: CHECK DIVISION BY ZERO
    uint16_t quotient = dividend/divisor;
    uint16_t remainder = dividend % abs(divisor);
    
    dstOp.value = quotient;
    dstOp.value |= remainder<<16;
    write_operand(dstOp, LONG);
    
    cpu->sr.ccr.negative = ( (int16_t) quotient) < 0;
    cpu->sr.ccr.zero = (quotient == 0);
    cpu->sr.ccr.overflow = 0;
    cpu->sr.ccr.carry = 0;
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
    printf("(SUBX)\n");
    uint8_t size = ins.f3;
    uint8_t x = cpu->sr.ccr.extend;

    operand srcOp, dstOp;
    if (ins.f4 & 1) { // Memory predecrement: -(Ax) - -(Ay) - X -> -(Ax)
        srcOp = read_operand(size, 0b100, ins.f5, false);
        dstOp = read_operand(size, 0b100, ins.f1, false);
    } else { // Data register: Dx - Dy - X -> Dx
        srcOp = read_operand(size, 0b000, ins.f5, false);
        dstOp = read_operand(size, 0b000, ins.f1, false);
    }

    uint32_t sign_bit;
    uint64_t mask;
    if (size == BYTE)      { sign_bit = 0x80;       mask = 0xFF; }
    else if (size == WORD) { sign_bit = 0x8000;     mask = 0xFFFF; }
    else                   { sign_bit = 0x80000000; mask = 0xFFFFFFFF; }

    uint64_t src_m = srcOp.value & mask;
    uint64_t dst_m = dstOp.value & mask;
    uint64_t wide_sub = src_m + x;       // effective subtrahend: src + X
    uint32_t result = (uint32_t)(dst_m - wide_sub);
    uint8_t carry = (wide_sub > dst_m);  // borrow occurred

    // Overflow: src and dst have different signs, and result sign differs from dst
    uint8_t overflow = ((srcOp.value & sign_bit) != (dstOp.value & sign_bit)) &&
                       ((result & sign_bit) != (dstOp.value & sign_bit));

    cpu->sr.ccr.carry = carry;
    cpu->sr.ccr.extend = carry;
    cpu->sr.ccr.overflow = overflow;
    cpu->sr.ccr.negative = (truncate_val(result, size) < 0);
    // Z is only cleared, never set — designed for multi-precision chains
    if (truncate_val(result, size) != 0)
        cpu->sr.ccr.zero = 0;

    dstOp.value = result;
    write_operand(dstOp, size);
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
    uint8_t size = ins.f3;
    operand srcOp = read_operand(size, 0b100, ins.f5, false); // Always predecrement mode
    operand dstOp = read_operand(size, 0b100, ins.f1, false);
    
    int32_t res = truncate_val(dstOp.value - srcOp.value, size);
    
    cpu->sr.ccr.negative = (res < 0);
    cpu->sr.ccr.zero = (res == 0);
    cpu->sr.ccr.overflow = check_overflow(srcOp.value, dstOp.value, res, size);
    cpu->sr.ccr.carry = check_carry(srcOp.value, dstOp.value, res, size, true);
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
