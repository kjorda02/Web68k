#pragma once
    #include "cpu.h"

void decode_op0(INS i, CPU* cpu);
void decode_op4(INS i, CPU* cpu);
void decode_op5(INS i, CPU* cpu);
void decode_op8(INS i, CPU* cpu);
void decode_op9(INS i, CPU* cpu);
void decode_op11(INS i, CPU* cpu);
void decode_op12(INS i, CPU* cpu);
void decode_op13(INS i, CPU* cpu);
void decode_op14(INS i, CPU* cpu);

void ori(INS31233 ins, CPU* cpu);
void andi(INS31233 ins, CPU* cpu);
void subi(INS31233 ins, CPU* cpu);
void addi(INS31233 ins, CPU* cpu);
void eori(INS31233 ins, CPU* cpu);
void cmpi(INS31233 ins, CPU* cpu);
void bop(INS31233 Ins, CPU* cpu);
void movep(INS ins, CPU* cpu);

void move(INS ins, CPU* cpu);

void move_from_sr(INS4233 ins, CPU* cpu);
void move_to_ccr(INS4233 ins, CPU* cpu);
void move_to_sr(INS4233 ins, CPU* cpu);
void negx(INS4233 ins, CPU* cpu);
void clr(INS4233 ins, CPU* cpu);
void neg(INS4233 ins, CPU* cpu);
void NOT(INS4233 ins, CPU* cpu);
void ext(INS4233 ins, CPU* cpu);
void nbcd(INS4233 ins, CPU* cpu);
void swap(INS4233 ins, CPU* cpu);
void pea(INS4233 ins, CPU* cpu);
void illegal(CPU* cpu);
void tas(INS4233 ins, CPU* cpu);
void tst(INS4233 ins, CPU* cpu);
void trap(INS84 ins, CPU* cpu);
void link(INS4233 ins, CPU* cpu);
void unlk(INS4233 ins, CPU* cpu);
void move_usp(INS4233 ins, CPU* cpu);
void reset(CPU* cpu);
void nop(CPU* cpu);
void stop(CPU* cpu);
void rte(CPU* cpu);
void rts(CPU* cpu);
void trapv(CPU* cpu);
void rtr(CPU* cpu);
void jsr(INS4233 ins, CPU* cpu);
void jmp(INS4233 ins, CPU* cpu);
void movem(INS4233 ins, CPU* cpu);
void lea(INS3333 ins, CPU* cpu);
void chk(INS3333 ins, CPU* cpu);

void addq(INS31233 ins, CPU* cpu);
void subq(INS31233 ins, CPU* cpu);
void Scc(INS4233 ins, CPU* cpu);
void DBcc(INS4233 ins, CPU* cpu);

void Bcc(INS ins, CPU* cpu);
void bsr(INS48 ins, CPU* cpu);

void moveq(INS ins, CPU* cpu);

void divu(INS31233 ins, CPU* cpu);
void divs(INS31233 ins, CPU* cpu);
void sbcd(INS31233 ins, CPU* cpu);
void OR(INS31233 ins, CPU* cpu);

void sub(INS31233 ins, CPU* cpu);
void subx(INS31233 ins, CPU* cpu);
void suba(INS31233 ins, CPU* cpu);

void eor(INS31233 ins, CPU* cpu);
void cmpm(INS31233 ins, CPU* cpu);
void cmp(INS31233 ins, CPU* cpu);
void cmpa(INS31233 ins, CPU* cpu);

void mult(INS31233 ins, CPU* cpu, bool sign);
void abcd(INS31233 ins, CPU* cpu);
void exg(INS31233 ins, CPU* cpu);
void AND(INS31233 ins, CPU* cpu);

void add(INS31233 ins, CPU* cpu);
void addx(INS31233 ins, CPU* cpu);
void adda(INS31233 ins, CPU* cpu);

void shiftop(INS31233 ins, CPU* cpu, uint8_t type);