#include "cpu.h"
CPU cpu;

void initCpu() {
    for (int i = 0; i < 8; i++) {
        cpu.d[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        cpu.a[i] = 0;
    }
    cpu.pc = cpu.sr = 0;

    memset(cpu.ram, 0, sizeof(cpu.ram));
}