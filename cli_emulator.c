#include "cli_emulator.h"

int main(int argc, char* argv[]) {
    if (argc < 2)
        fprintf(stderr, "ERROR: File to be run must be specified");
    else if (argc > 2)
        fprintf(stderr, "ERROR: Too many arguments.");

    CPU* cpu = initCpu(); // Initializes register and RAM with 0s
    load_srec(argv[1], cpu); // Reads S-record file and loads instructions to memory
}