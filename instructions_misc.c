#include "instructions.h"

void move(INS ins, CPU* cpu) {
    printf("MOVE\n");
}

void moveq(INS i, CPU* cpu) {
    printf("MOVEQ\n");
    INS318 ins = *(INS318*) &i;
}

void Bcc(INS i, CPU* cpu) {
    INS48 ins = *(INS48*) &i;
}