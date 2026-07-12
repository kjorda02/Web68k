#include <emscripten.h>
#include "util.h"
#include "colors.h"

static CPU cpu;

EMSCRIPTEN_KEEPALIVE
CPU* initCpu() {
    for (int i = 0; i < 8; i++) {
        cpu.d[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        cpu.a[i] = 0;
    }
    CCR ccr = {0,0,0,0,0,0};
    SR sr = {ccr, 0, 0, 1, 0, 0};
    cpu.sr = sr;
    cpu.pc = cpu.cycles = 0;
    cpu.recursion_level = 0;
    cpu.step_start_level = STEP_LEVEL_UNSET;
    cpu.a[7] = 0x01000000; // User stack pointer

    memset(cpu.ram, 255, sizeof(cpu.ram));
    memset(cpu.breakpoints, 0, sizeof(cpu.breakpoints));

    return &cpu;
}

EMSCRIPTEN_KEEPALIVE
void* wasm_malloc(size_t n) {
    return malloc(n);
}

EMSCRIPTEN_KEEPALIVE
void wasmfree(void* ptr) {
    free(ptr);
}

EMSCRIPTEN_KEEPALIVE
uint32_t load_program(char* srec) {
    //printf("Loaded srec:\n %s\n", srec);

    uint32_t entryPoint = process_records(srec, &cpu);
    cpu.pc = entryPoint;
    return entryPoint;
}

EMSCRIPTEN_KEEPALIVE
void set_breakpoint(uint32_t addr, bool val) {
    printf("breakpoint @%X: %d\n", addr, val);
    cpu.breakpoints[addr] = val;
}

/* --- START_PROGRAM -----------------------------------------------------------------------------------
 * RUNS THE MAIN FETCH-DECODE-EXECUTE LOOP, CALLING HELPER METHODS TO ACTUALLY PROCESS THE INSTRUCTIONS
*/
// uint32_t run_program() {
//     INS IR = {0, 0}; // Instruction register

//     while (IR.opcode != 0b1111) {
//         IR = fetch();
//         decode(IR); // Decode calls execute
//     }
//     printf("(SENTINEL)\n");

//     return cpu.pc;
// }


EMSCRIPTEN_KEEPALIVE
bool run_burst(int cycles, int mode) {
    INS IR = {0, 0}; // Instruction register
    int initial_cycles = cpu.cycles;

    if (cpu.step_start_level == STEP_LEVEL_UNSET)
        cpu.step_start_level = cpu.recursion_level;

    while (IR.opcode != 0b1111 && (cpu.cycles - initial_cycles) < cycles) {
        if (cpu.breakpoints[cpu.pc]) {
            cpu.step_start_level = STEP_LEVEL_UNSET;
            return true;
        }

        IR = fetch();
        decode(IR); // Decode calls execute
        cpu.cycles++;

        if ( (mode==STEP_OVER && cpu.recursion_level <= cpu.step_start_level)
        || (mode==STEP_INTO)
        || (mode==STEP_OUT && cpu.recursion_level < cpu.step_start_level)) {
            cpu.step_start_level = STEP_LEVEL_UNSET;
            return true;
        }
    }

    return false;
}

/* --- STEP_FORWARDS -------------------------------------------------------------------------------------
 * Runs the instruction currently pointed to by the PC. Returns the new value of the PC.
*/
EMSCRIPTEN_KEEPALIVE
uint32_t step_forwards() {
    INS IR = fetch();
    if (IR.opcode != 0b1111) {
        decode(IR);
    }
    else {
        printf("(SENTINEL)\n");
    }

    return cpu.pc;
}

/* --- FETCH -----------------------------------------------------------------------------------
 * Obtains the instruction pointed to by the PC and increments it by 2 (2 bytes per instruction)
*/
INS fetch() {
    uint16_t word = fetch_data(WORD);
    //printf(MAGENTA"DECODING WORD @" BOLD_GREEN "%X" MAGENTA ": %X " BOLD_GREEN, cpu.pc-2, word);
    // printf("DECODING WORD @%X: %X ", cpu.pc-2, word);
    INS i;
    memcpy(&i, &word, 2);

    return i;
}

/* --- READ_MEM --------------------------------------------------------------------------------
 * Reads the memory array, which is in big-endian format. It is necessary to reverse the order
 * of the bytes. <pos> indicates the memory position to be read, and <size> the size of the data
 * to be read (Byte=00, Word=01, Long=10).
*/
uint32_t read_mem(uint32_t pos, uint8_t size) {
    if (pos < 0 || pos > sizeof(cpu.ram)-1) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Cannot read from position 0x%x: Out of bounds\n", pos);
        logmsg(ERROR, "cpu.c:read_mem", buffer);
        exit(EXIT_FAILURE);
    }
    // We need to convert it to little endian format before casting to uint32_t
    uint8_t bytes[4] = {cpu.ram[pos+3],cpu.ram[pos+2],cpu.ram[pos+1], cpu.ram[pos]};
    uint32_t data;
    memcpy(&data, &bytes, 4);

    if (size == BYTE) {
        data >>= 24;
        return (int8_t) (data & 0x000000FF); // Casting to signed int for sign extension
    }
    else if (size == WORD) {
        data >>= 16;
        data = (int16_t) (data & 0x0000FFFF);
        return data;
    }
    else if (size == LONG) {
        return data;
    }
    else {
        logmsg(ERROR, "cpu.c:read_mem", "Invalid size parameter");
        exit(EXIT_FAILURE);
    }
}


/* --- READ_MEM_WINDOW --------------------------------------------------------------------------
 * Allows access to the cpu's memory for displaying in the UI. We use windowSize to do some 
 * bounds checking, and then return a pointer to the cpu's memory array at the position requested'
 */
EMSCRIPTEN_KEEPALIVE
uint8_t* read_mem_window(uint32_t pos, uint16_t windowSize) {
    if (pos < 0 || pos+windowSize > sizeof(cpu.ram)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Cannot read window [0x%x, 0x%x]: Out of bounds\n", pos, pos+windowSize-1);
        logmsg(ERROR, "cpu.c:read_mem_window", buffer);
        exit(EXIT_FAILURE);
    }
    
    return &cpu.ram[pos];
}

/* --- WRITE_MEM --------------------------------------------------------------------------------
 * Writes to the memory array, which is in big-endian format. <pos> indicates the memory position
 * to be written to, and <size> the size of the data to be written (Byte=00, Word=01, Long=10).
*/
EMSCRIPTEN_KEEPALIVE
void write_mem(uint32_t pos, uint8_t size, uint32_t data) {
    // printf("write_mem: pos: %X, size: %d, data: %X\n", pos, size, data);
    if (pos > sizeof(cpu.ram)-1) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Cannot write to position 0x%x: Out of bounds\n", pos);
        logmsg(ERROR, "cpu.c:write_mem", buffer);
        exit(EXIT_FAILURE);
    }
    if (size == BYTE) // Byte size. Write least significant byte to the position
        cpu.ram[pos]   =  data & 0xFF;

    else if (size == WORD){ // Word size. Now the memory position contains the second-to-last LSB
        cpu.ram[pos]   = (data >> 8) & 0xFF; // And the LSB is in the next position
        cpu.ram[pos+1] =  data & 0xFF;
    }
    else if (size == LONG) { // And so on
        cpu.ram[pos]   = (data >> 24) & 0xFF;
        cpu.ram[pos+1] = (data >> 16) & 0xFF;
        cpu.ram[pos+2] = (data >> 8) & 0xFF;
        cpu.ram[pos+3] =  data & 0xFF;
    }
    else {
        logmsg(ERROR, "cpu.c:write_mem", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
}

uint32_t fetch_data(uint8_t size) {
    uint32_t data;
    if (size == BYTE) { // Byte
        data = read_mem(cpu.pc, WORD);
        data &= 0x00FF; // Byte is stored in low part of word
        data = (int8_t) data; // Sign extend
    }
    else
        data = read_mem(cpu.pc, size);

    cpu.pc += 2; // Incremented at least 2 (for byte and word operations)
    if (size == LONG)
        cpu.pc += 2; // Another 2 if long
    else if (size > LONG){
        logmsg(ERROR, "cpu.c:fetch_data", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
    return data;
}

/* --- WRITE_DN ----------------------------------------------------------------------------------
 * Writes data to a data register, taking into account that if a byte or word is written, the
 * remaining more significant bytes must remain unchanged (NO SIGN EXTENSION!).
*/
EMSCRIPTEN_KEEPALIVE
void write_Dn(uint32_t data, uint8_t n, uint8_t size) {
    // printf("data: %d, n: %d\n", data, n);
    uint8_t bytes = size_to_bytes(size);
    uint32_t mask = 0xFFFFFFFF;
    mask >>= (4 - bytes)*8; // 0x000000FF for byte, 0x0000FFFF for word, and 0xFFFFFFFF for long
    uint32_t mask2 = ~mask; // Invert all bits

    data &= mask; // Just in case, clear the higher bytes that don't contain data
    cpu.d[n] &= mask2; // Clear all the bytes that are to be modified
    cpu.d[n] |= data; // Set all the 1s that are set in the data in the register
}

EMSCRIPTEN_KEEPALIVE
void write_An(uint32_t data, uint8_t n, uint8_t size) {
    if (size == BYTE) {
        logmsg(ERROR, "cpu.c:write_An", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
    cpu.a[n] = truncate_val(data, size);
}

uint32_t read_Dn(uint8_t n, uint8_t size) {
    return truncate_val(cpu.d[n], size); // Clears higher bits and sign extends
}

EMSCRIPTEN_KEEPALIVE
uint32_t* read_D_regs() {
    return cpu.d;
}

uint32_t read_An(uint8_t n, uint8_t size) {
    if (size == BYTE) {
        logmsg(ERROR, "cpu.c:read_An", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
    return truncate_val(cpu.a[n], size);
}

EMSCRIPTEN_KEEPALIVE
uint32_t* read_A_regs() {
    return cpu.a;
}

EMSCRIPTEN_KEEPALIVE
void write_pc(uint32_t val) { // Only used by js code
    cpu.pc = val;
}

EMSCRIPTEN_KEEPALIVE
uint32_t read_pc() { // Only used by js code
    return cpu.pc;
}

EMSCRIPTEN_KEEPALIVE
void write_sr(uint16_t val) { // Only used by js code
    memcpy(&cpu.sr, &val, 2);
}

EMSCRIPTEN_KEEPALIVE
uint16_t read_sr() {
    uint16_t val;
    memcpy(&val, &cpu.sr, 2);
    return val;
}

EMSCRIPTEN_KEEPALIVE
unsigned long read_cycles() {
    return cpu.cycles;
}

/* --- DECODE ----------------------------------------------------------------------------------
 * Decodes opcode and calls appropiate function for further decoding
*/
void decode(INS ins) {
    static void (*execute[16])(INS, CPU*) = { // Array of function pointers
        &decode_op0,
        &move,
        &move,
        &move,
        &decode_op4,
        &decode_op5,
        &Bcc,
        &moveq,
        &decode_op8,
        &decode_op9,
        NULL, // not used
        &decode_op11,
        &decode_op12,
        &decode_op13,
        &decode_op14,
        NULL, // not used
    };

    if (execute[ins.opcode] != NULL){
        execute[ins.opcode](ins, &cpu);  // Calls the function corresponding to the opcode
    }
}

/* --- READ_OPERAND ----------------------------------------------------------------------------------
 * Finds the effective address using the values of (M) and (Xn). Reads the data, if (adressOnly) is  not
 * set. (size) specifies the size of the data to be read (00=Byte, 01=Word, 10=Long). Returns an operand
 * struct, that contains the effective address, and the value of the operand if it was read (ALWAYS SIGN EXTENDED).
 * It also contains a boolean which indicates whether the data was in memory or in a register. Calling the function
 * with (adressOnly) unset is useful to find the EA of the destination operand, when it does not need to be read.
 *
 * WARNING: EXTENSION WORDS AFTER THE INSTRUCTION MUST BE READ IN THE FOLLOWING ORDER:
 *      [1] Instruction specific data (e.g. bit number for BTST, displacement for BRA or immediate data for ADDI)
 *      [2] Data relating to the first operand's adressing mode (immediate/indirect/brief extension word)
 *      [2] Data relating to the second operand's adressing mode (indirect/brief extension word)
*/
operand read_operand(uint8_t size, uint8_t M, uint8_t Xn, bool addressOnly) {
    uint8_t nbytes = size_to_bytes(size); // Used for postincrement and predecrement

    // B, W and L refer to displacements or adresses: 8, 16 or 32 bit, respectively. An refers to an adress register.
    // Dn refers to a data register. Xn referes to either. S refers to the size of the operand in bytes (1, 2 or 4)
    operand op;
    op.mem_access = true;
    uint16_t ext_word;
    BEW bew; // Brief extension word
    switch(M){
        case 0b000: // Dn / <ea> = Dn / Data register direct
            op.mem_access = false;
            op.dataReg = true;
            op.n = Xn;
            if (!addressOnly)
                op.value = read_Dn(Xn, size);
            break;
        case 0b001: // An / <ea> = An / Adress register direct
            op.dataReg = false;
            op.n = Xn;
            op.mem_access = false;
            if (!addressOnly)
                op.value = read_An(Xn, size);
            break;
        case 0b010: // (An) / <ea> = [An] / Adress register indirect
            op.address = cpu.a[Xn];
            break;
        case 0b011: // (An)+ / <ea> = [An] ; An <- [An] + S / Indirect with postincrement
            // TODO: If register is A7 (stack pointer), it is always kept aligned (never add only 1)
            op.address = cpu.a[Xn];
            cpu.a[Xn] += nbytes;
            // TODO: Modify flags for postincrement and predecrement
            break;
        case 0b100: // -(An) / An <- [An] - S ; <ea> = [An] / Indirect with predecrement
            cpu.a[Xn] -= nbytes;
            op.address = cpu.a[Xn];
            break;
        case 0b101: // W(An) / <ea> = W + [An] / Indirect with displacement
            op.address = fetch_data(WORD) + cpu.a[Xn];
            break;
        case 0b110: // B(An, Xn.S) / <ea> = B + [An] + [Xn] / Indirect with index
            ext_word = fetch_data(WORD);
            memcpy(&bew, &ext_word, 2);
            op.address = ((int8_t) bew.displacement) + cpu.a[Xn];

            uint8_t s;
            if (bew.lon)
                s = LONG;
            else
                s = WORD;

            if (bew.mode) // mode=1 -> An, mode=0 -> Dn
                op.address += read_An(bew.n, s);
            else
                op.address += read_Dn(bew.n, s);
            break;
        case 0b111:
            switch(Xn) {
                case 0b000: // W / <ea> = [W] / Absolute
                        op.address = fetch_data(WORD);
                    break;
                case 0b001: // L / <ea> = [L] / Absolute long
                        op.address = fetch_data(LONG);
                    break;
                case 0b010: // W(PC) / <ea> = W + [PC] / relative
                        op.address = cpu.pc; // Important that we store the PC before fetching the displacement
                        op.address += fetch_data(WORD); // As this will increment the PC
                        break;
                case 0b011: // B(PC, Xn.S) / <ea> = B + [PC] + [Xn] / relative with offset
                    op.address = cpu.pc;
                    ext_word = fetch_data(WORD);
                    memcpy(&bew, &ext_word, 2);
                    op.address += ((int8_t) bew.displacement);

                    uint8_t s;
                    if (bew.lon)
                        s = LONG;
                    else
                        s = WORD;

                    if (bew.mode) // mode=1 -> An, mode=0 -> Dn
                        op.address += read_An(bew.n, s);
                    else
                        op.address += read_Dn(bew.n, s);
                    break;
                case 0b100: // #k / <ea> = [PC] ; PC <- [PC] + S / Immediate
                    op.mem_access = false;
                    op.value = fetch_data(size);
                    break;
            }
            break;
    }

    if (op.mem_access && !addressOnly) { // If it wasn't a direct register operation, we need to read the value from memory
        // fprintf(stderr, "Effective address calculated: %x\n", op.address);
        op.value = read_mem(op.address, size);
    }
    return op;
}

/* --- WRITE_OPERAND ----------------------------------------------------------------------------------
 * This function simply writes the data in the operand struct to memory or to the appropiate register.
 * read_operand is used to obtain the operand struct, which indicates whether we need to write to a
 * register or to memory, and the effective address in the later case.
*/
void write_operand(operand op, uint8_t size) {
    if (op.mem_access)
        write_mem(op.address, size, op.value);
    else {
        if (op.dataReg)
            write_Dn(op.value, op.n, size);
        else
            write_An(op.value, op.n, size);
    }
}