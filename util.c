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
        case 0b00: // Byte
            return 1;
        case 0b01: // Word
            return 2;
        case 0b10: // Long
            return 4;
        default:
            logmsg(ERROR, "util.c:size_to_bytes", "Invalid size argument");
            exit(EXIT_FAILURE);
    }
}

/* --- TRUNCATE_VALUE -----------------------------------------------------------------------
 * Clears the unused higher bits and sign-extends if necessary
*/
// uint32_t truncate_value(uint32_t data, uint8_t size) {
//
// }

// Returns 0 for positive, 1 for negative
uint8_t get_sign(uint32_t val, uint8_t size) {
    unsigned int displ = 8*size_to_bytes(size) - 1;
    return val & (1 << displ);
}

uint8_t check_overflow(uint32_t a, uint32_t b, uint32_t res, uint8_t size) {
    uint8_t sign1 = get_sign(a, size);

    // If both operands have the same sign but the result has different sign
    if (sign1==get_sign(b, size) && get_sign(res, size) != sign1)
        return 1;
    else
        return 0;
}

uint8_t check_carry(uint32_t a, uint32_t b, uint32_t res, uint8_t size, bool substraction) {
    if (size == 0b00)  // Byte
        res &= 0x000000FF;
    else if (size == 0b01)  // Word
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
    cpu->sr.ccr.overflow = check_overflow(src, dst, res, size);
    cpu->sr.ccr.carry = check_carry(src, dst, res, size, false);
    cpu->sr.ccr.extend = cpu->sr.ccr.carry;
    cpu->sr.ccr.negative = get_sign(res, size);
    cpu->sr.ccr.zero = (res == 0);
}

void set_flags_sub(uint32_t src, uint32_t dst, uint8_t size, CPU* cpu) {
    uint32_t res = dst - src;
    cpu->sr.ccr.overflow = check_overflow(-src, dst, res, size);
    cpu->sr.ccr.carry = check_carry(src, dst, res, size, true);
    cpu->sr.ccr.extend = cpu->sr.ccr.carry;
    cpu->sr.ccr.negative = get_sign(dst, size);
    cpu->sr.ccr.zero = (dst == 0);
}