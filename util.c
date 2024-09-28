#include "util.h"
#include "colors.h"

void logmsg(int8_t status, char* func, char* message) { // For now, we just print to stderr
    if (status == INFO)
        fprintf(stderr, BOLD_GREEN "[INFO] " BOLD_MAGENTA "(%s) > " GREEN "%s\n" RES, func, message);

    else if (status == WARNING)
        fprintf(stderr, BOLD_YELLOW "[WARNING] " BOLD_MAGENTA "(%s) > " YELLOW "%s\n" RES, func, message);
    else if (status == ERROR)
        fprintf(stderr, BOLD_RED "[ERROR] " BOLD_MAGENTA "(%s) > " RED "%s\n" RES, func, message);
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