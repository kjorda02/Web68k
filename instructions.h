#include "cpu.h"

void decode_op0(INS i, CPU cpu);
void decode_op4(INS i, CPU cpu);

//void ori_to_ccr(INS ins, CPU cpu);
//void ori_to_sr(INS ins, CPU cpu);
void ori(INS31233 ins, CPU cpu);
//void andi_to_ccr(INS ins, CPU cpu);
//void andi_to_sr(INS ins, CPU cpu);
void andi(INS31233 ins, CPU cpu);
void subi(INS31233 ins, CPU cpu);
void addi(INS31233 ins, CPU cpu);
//void eori_to_ccr(INS ins, CPU cpu);
//void eori_to_sr(INS ins, CPU cpu);
void eori(INS31233 ins, CPU cpu);
void cmpi(INS31233 ins, CPU cpu);
//void btst(INS ins, CPU cpu);
//void bchg(INS ins, CPU cpu);
//void bclr(INS ins, CPU cpu);
//void bset(INS ins, CPU cpu);
void bop(INS31233 Ins, CPU cpu);
void movep(INS ins, CPU cpu);

//void movea(INS ins, CPU cpu);
void move(INS ins, CPU cpu);

void move_from_sr(INS4233 ins, CPU cpu);
void move_to_ccr(INS4233 ins, CPU cpu);
void move_to_sr(INS4233 ins, CPU cpu);
void negx(INS4233 ins, CPU cpu);
void clr(INS4233 ins, CPU cpu);
void neg(INS4233 ins, CPU cpu);
void NOT(INS4233 ins, CPU cpu);
void ext(INS4233 ins, CPU cpu);
void nbcd(INS4233 ins, CPU cpu);
void swap(INS4233 ins, CPU cpu);
void pea(INS4233 ins, CPU cpu);
void illegal(CPU cpu);
void tas(INS4233 ins, CPU cpu);
void tst(INS4233 ins, CPU cpu);
void trap(INS84 ins, CPU cpu);
void link(INS4233 ins, CPU cpu);
void unlk(INS4233 ins, CPU cpu);
void move_usp(INS4233 ins, CPU cpu);
void reset(CPU cpu);
void nop(CPU cpu);
void stop(CPU cpu);
void rte(CPU cpu);
void rts(CPU cpu);
void trapv(CPU cpu);
void rtr(CPU cpu);
void jsr(INS ins, CPU cpu);
void jmp(INS ins, CPU cpu);
void movem(INS4233 ins, CPU cpu);
void lea(INS ins, CPU cpu);
void chk(INS ins, CPU cpu);

void addq(INS ins, CPU cpu);
void subq(INS ins, CPU cpu);
void Scc(INS ins, CPU cpu);
void DBcc(INS ins, CPU cpu);

void bra(INS ins, CPU cpu);
void bsr(INS ins, CPU cpu);
void Bcc(INS ins, CPU cpu);

void moveq(INS ins, CPU cpu);

void divu(INS ins, CPU cpu);
void divs(INS ins, CPU cpu);
void sbcd(INS ins, CPU cpu);
void OR(INS ins, CPU cpu);

void sub(INS ins, CPU cpu);
void subx(INS ins, CPU cpu);
void suba(INS ins, CPU cpu);

void eor(INS ins, CPU cpu);
void cmpm(INS ins, CPU cpu);
void cmp(INS ins, CPU cpu);
void cmpa(INS ins, CPU cpu);

void mulu(INS ins, CPU cpu);
void muls(INS ins, CPU cpu);
void abcd(INS ins, CPU cpu);
void exg(INS ins, CPU cpu);
void AND(INS ins, CPU cpu);

void add(INS ins, CPU cpu);
void addx(INS ins, CPU cpu);
void adda(INS ins, CPU cpu);

void ASd(INS ins, CPU cpu);
void LSd(INS ins, CPU cpu);
void ROXd(INS ins, CPU cpu);
void ROd(INS ins, CPU cpu);