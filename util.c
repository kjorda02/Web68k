#include "util.h"
#include "colors.h"

void logmsg(int8_t status, char* func, char* message) { // For now, we just print to stderr
    if (status == INFO)
        fprintf(stderr, BOLD_GREEN "[INFO] " BOLD_GREEN "(%s) > " GREEN "%s\n" RES, func, message);

    else if (status == WARNING)
        fprintf(stderr, BOLD_YELLOW "[WARNING] " BOLD_GREEN "(%s) > " YELLOW "%s\n" RES, func, message);
    else if (status == ERROR)
        fprintf(stderr, BOLD_RED "[ERROR] " BOLD_GREEN "(%s) > " RED "%s\n" RES, func, message);
}

uint8_t size_to_bytes(uint8_t size) {
    switch(size) {
        case BYTE:
            return 1;
        case WORD:
            return 2;
        case LONG:
            return 4;
        default:
            logmsg(ERROR, "util.c:size_to_bytes", "Invalid size argument");
            exit(EXIT_FAILURE);
    }
}

/* --- TRUNCATE_VALUE -----------------------------------------------------------------------
 * Clears the unused higher bits and sign-extends the data
*/
uint32_t truncate_val(uint32_t data, uint8_t size) {
    if (size == BYTE) { // Byte
        return (int8_t) (data & 0x000000FF); // Casting to signed int for sign extension
    }
    else if (size == WORD) {
        return (int16_t) (data & 0x0000FFFF);
    }
    else if (size == LONG) {
        return data;
    }
    else {
        logmsg(ERROR, "util.c:truncate_val", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
}

uint8_t check_overflow(uint32_t a, uint32_t b, uint32_t res, uint8_t size) {
    bool sign_a = ((int32_t) a) < 0;
    bool sign_b = ((int32_t) b) < 0;
    bool sign_res = ((int32_t) res) < 0;

    // If both operands have the same sign but the result has different sign
    return (sign_a == sign_b && sign_res != sign_a);
}

uint8_t check_carry(uint32_t a, uint32_t b, uint32_t res, uint8_t size, bool substraction) {
    if (size == BYTE)
        res &= 0x000000FF;
    else if (size == WORD)
        res &= 0x0000FFFF;

    // In a sum, carry occurs if the result is smaller than both operands (unsigned comparison)
    if (!substraction && res < a && res < b)
        return 1;
    // In a subtraction, it occurs when the result is larger than both operands
    else if (substraction && res > a && res > b)
        return 1;
    else
        return 0;
}

void set_flags_add(uint32_t src, uint32_t dst, uint8_t size, CPU* cpu) {
    uint32_t res = src + dst;
    res = truncate_val(res, size); // Truncate and sign extend

    cpu->sr.ccr.overflow = check_overflow(src, dst, res, size);
    cpu->sr.ccr.carry = check_carry(src, dst, res, size, false);
    cpu->sr.ccr.extend = cpu->sr.ccr.carry;
    cpu->sr.ccr.negative = ((int32_t) res < 0);
    cpu->sr.ccr.zero = (res == 0);
}

void set_flags_sub(uint32_t src, uint32_t dst, uint8_t size, CPU* cpu) {
    uint32_t res = dst - src;
    res = truncate_val(res, size); // Truncate and sign extend

    cpu->sr.ccr.overflow = check_overflow(-src, dst, res, size);
    cpu->sr.ccr.carry = check_carry(src, dst, res, size, true);
    cpu->sr.ccr.extend = cpu->sr.ccr.carry;
    cpu->sr.ccr.negative = ((int32_t) res < 0);
    cpu->sr.ccr.zero = (res == 0);
}

bool check_condition(uint8_t condition, CCR ccr) {
    switch(condition) {
        case 0b0000: // True
            return true;
        case 0b0001: // False
            break;
        case 0b0010:
            return !(ccr.carry || ccr.zero); // HI (Higher)
        case 0b0011:
            return ccr.carry || ccr.zero; // LS (Lower or same)
        case 0b0100:
            return !ccr.carry; // HS/CC (Higher or same than / carry clear)
        case 0b0101:
            return ccr.carry; // LO/CS (Lower than / carry set)
        case 0b0110:
            return !ccr.zero; // NE (Not equal)
        case 0b0111:
            return ccr.zero; // EQ (Equal)
        case 0b1000:
            return !ccr.overflow; // VC (Overflow clear)
        case 0b1001:
            return ccr.overflow; // VS (Overflow set)
        case 0b1010:
            return !ccr.negative; // PL (Positive)
        case 0b1011:
            return ccr.negative; // MI (Negative)
        case 0b1100:
            return !(ccr.negative ^ ccr.overflow); // GE (Greater or equal)
        case 0b1101:
            return ccr.negative ^ ccr.overflow; // LT (Less than)
        case 0b1110:
            return !(ccr.zero | (ccr.negative ^ ccr.overflow)); // GT (Greater than)
        case 0b1111: // LE (Less or equal)
            return ccr.zero | (ccr.negative ^ ccr.overflow);
    }
    return false;
}