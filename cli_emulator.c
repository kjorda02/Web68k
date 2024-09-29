#include "cli_emulator.h"

int main(int argc, char* argv[]) {
    if (argc < 2)
        fprintf(stderr, "ERROR: File to be run must be specified");
    else if (argc > 2)
        fprintf(stderr, "ERROR: Too many arguments.");

    CPU* cpu = initCpu(); // Initializes register and RAM with 0s
    uint32_t entryPoint = load_srec(argv[1], cpu); // Reads S-record file and loads instructions to memory
    int statusCode = run_program(entryPoint);

    print_registers(cpu);
}

void print_registers(CPU* cpu) {
    printf(BLUE"╔═════════════════════════════════════════════════════════════════════╗\n");
    printf(BLUE"║" BOLD_MAGENTA"                       MOTOROLA 68000 REGISTERS                      "BLUE"║\n");
    printf(BLUE"╠════════════════╦═════════════════╦═════════════════╦════════════════╣\n");
    for (int i = 0; i < 4; i++) {
        printf(BLUE"║");
        printf(BOLD_GREEN " [D%d]=%08x" BLUE "  ║ ", i, cpu->d[i]);
        printf(BOLD_GREEN " [D%d]=%08x" BLUE "  ║ ", i+4, cpu->d[i+4]);
        printf(BOLD_GREEN " [A%d]=%08x" BLUE "  ║ ", i, cpu->a[i]);
        printf(BOLD_GREEN " [A%d]=%08x" BLUE" ║\n", i+4, cpu->a[i+4]);
        if (i < 3)
            printf(BLUE"╠════════════════╬═════════════════╬═════════════════╬════════════════╣\n");
        else
            printf(BLUE"╠════════════════╩═════════════════╩═════════════════╩════════════════╣\n");
    }
    printf(BLUE"║" BOLD_GREEN" [SR]=%016b"BLUE"                                               ║\n", *((uint16_t*) &cpu->sr));
    printf(BLUE"║" RED"      T S  INT   XNZVC                                               " BLUE"║\n");
    printf("╚═════════════════════════════════════════════════════════════════════╝\n"RES);
}