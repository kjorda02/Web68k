#include "instructions.h"
#include "util.h"

static CPU cpu;

CPU* initCpu() {
    for (int i = 0; i < 8; i++) {
        cpu.d[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        cpu.a[i] = 0;
    }
    CCR ccr = {0,0,0,0,0,0};
    SR sr = {ccr, 0, 0, 0, 0, 0};
    cpu.sr = sr;
    cpu.pc = cpu.cycles = 0;

    memset(cpu.ram, 0, sizeof(cpu.ram));

    return &cpu;
}

/* --- START_PROGRAM -----------------------------------------------------------------------------------
 * RUNS THE MAIN FETCH-DECODE-EXECUTE LOOP, CALLING HELPER METHODS TO ACTUALLY PROCESS THE INSTRUCTIONS
*/
int run_program(uint32_t entryPoint) {
    cpu.pc = entryPoint;
    INS IR = {0, 0}; // Instruction register

    while (IR.opcode != 0b1111) {
        IR = fetch();
        decode(IR); // Decode calls execute
    }

    return 0;
}

/* --- FETCH -----------------------------------------------------------------------------------
 * Obtains the instruction pointed to by the PC and increments it by 2 (2 bytes per instruction)
*/
INS fetch() {
    uint16_t word = fetch_data(WORD);
    printf("DECODING WORD: %x\n", word);
    INS i = *((INS*) &word);

    return i;
}

/* --- READ_MEM --------------------------------------------------------------------------------
 * Reads the memory array, which is in big-endian format. It is necessary to reverse the order
 * of the bytes. <pos> indicates the memory position to be read, and <size> the size of the data
 * to be read (Byte=00, Word=01, Long=10).
*/
uint32_t read_mem(uint32_t pos, uint8_t size) {

    // We need to convert it to little endian format before casting to uint32_t
    uint8_t bytes[4] = {cpu.ram[pos+3],cpu.ram[pos+2],cpu.ram[pos+1], cpu.ram[pos]};
    uint32_t data = *((uint32_t*) &bytes);

    if (size == 0b00) { // Byte
        return (int8_t) (data & 0x000000FF); // Casting to signed int for sign extension
    }
    else if (size == 0b01) { // Word
        return (int16_t) (data & 0x0000FFFF);
    }
    else if (size == 0b10) { // Long
        return data;
    }
    else {
        logmsg(ERROR, "cpu.c:read_mem", "Invalid size parameter");
        exit(EXIT_FAILURE);
    }
}

/* --- WRITES_MEM --------------------------------------------------------------------------------
 * Writes to the memory array, which is in big-endian format. <pos> indicates the memory position
 * to be written to, and <size> the size of the data to be written (Byte=00, Word=01, Long=10).
*/
void write_mem(uint32_t pos, uint8_t size, uint32_t data) {

    if (size == 0b00) // Byte size. Write least significant byte to the position
        cpu.ram[pos]   =  data & 0xFF;

    else if (size == 0b01){ // Word size. Now the memory position contains the second-to-last LSB
        cpu.ram[pos]   = (data >> 8) & 0xFF; // And the LSB is in the next position
        cpu.ram[pos+1] =  data & 0xFF;
    }
    else if (size == 0b10) { // And so on
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
    uint32_t data = read_mem(cpu.pc, size);
    if (size == 0b00 || size == 0b01)
        cpu.pc += 2;
    else if (size == 0b10)
        cpu.pc += 4;
    else {
        logmsg(ERROR, "cpu.c:fetch_data", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
    return data;
}

/* --- WRITE_DN ----------------------------------------------------------------------------------
 * Writes data to a data register, taking into account that if a byte or word is written, the
 * remaining more significant bytes must remain unchanged.
*/
void write_Dn(uint32_t data, uint8_t n, uint8_t size) {
    uint8_t bytes = size_to_bytes(size);
    uint32_t mask = 0xFFFFFFFF;
    mask >>= (4 - bytes)*8; // 0x000000FF for byte, 0x0000FFFF for word, and 0xFFFFFFFF for long
    uint32_t mask2 = ~mask; // Invert all bits

    data &= mask; // Just in case, clear the higher bytes that don't contain data'
    cpu.d[n] &= mask2; // Clear all the bytes that are to be modified
    cpu.d[n] |= data; // Set all the 1s that are set in the data in the register
}

void write_An(uint32_t data, uint8_t n, uint8_t size) {
    uint32_t dataext = data;
    if (size == 0b01) { // Word
        dataext &= 0x0000FFFF;
        dataext = (int16_t) dataext; // SIGN EXTEND VERY IMPORTANT!
    }
    else if (size != 0b10) {
        logmsg(ERROR, "cpu.c:read_An", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
    cpu.a[n] = dataext;
}

uint32_t read_Dn(uint8_t n, uint8_t size) {
    if (size == 0b00) { // Byte
        return (int8_t) (cpu.d[n] & 0x000000FF); // Casting to signed int for sign extension
    }
    else if (size == 0b01) { // Word
        return (int16_t) (cpu.d[n] & 0x0000FFFF);
    }
    else if (size == 0b10) { // Long
        return cpu.d[n];
    }
    else {
        logmsg(ERROR, "cpu.c:read_Dn", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
}

uint32_t read_An(uint8_t n, uint8_t size) {
    if (size == 0b01) { // Word
        return (int16_t) (cpu.d[n] & 0x0000FFFF); // Casting to signed int for sign extension
    }
    else if (size == 0b10) { // Long
        return cpu.d[n];
    }
    else {
        logmsg(ERROR, "cpu.c:read_An", "Invalid size argument");
        exit(EXIT_FAILURE);
    }
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

/* --- ACCESS_OPERAND ----------------------------------------------------------------------------------
 * Finds the effective address according to the mode specified in (M) and (Xn). Writes the data
 * in (buf), taking into account the size specified in (size). 0=Byte, 1=Word, 2=Long.
 * (allowed_addr_modes) indicates the valid values for (M) and (Xn). Since these 2 parameters are 3-bit,
 * they only have 8 possible values each. The low byte of (allowed_addr_modes) indicates which values are
 * allowed for (M), with the least significant bit indicating whether M=0 is allowed, bit 1 indicating if
 * M=1 is allowed, and so on. If M=7, then it is necessary to look at (Xn), and for this, the high byte
 * is used to determine the allowed values for (Xn) in the same manner.
 * TODO: CHANGE NAME
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MOTOROLA 68000 INSTRUCTION FORMAT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *      [1] SINGLE EFFECTIVE ADDRESS OPERATION WORD (1 word)
 *      [2] IMMEDIATE OPERAND OR SOURCE BRIEF EXTENSION WORD
 *      [3] DESTINATION BRIEF EXTENSION WORD
 *
*/
int access_operand(uint32_t* buf, uint8_t size, uint16_t allowed_addr_modes, uint8_t M, uint8_t Xn, bool write) {
    if ( (1 & (allowed_addr_modes >> M) ) == 0) { // Checks bit M from the low byte of allowed_addr_modes
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Adressing mode M=%b not allowed", M);
        logmsg(ERROR, "cpu.c:read_operand", buffer);
        exit(EXIT_FAILURE);
    }
    if (M == 7 && (0x100 & (allowed_addr_modes >> Xn)) == 0 ) { // Checks bit Xn from the high byte of allowed_addr_modes
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Adressing mode M=%b, Xn=%b not allowed", M, Xn);
        logmsg(ERROR, "cpu.c:read_operand", buffer);
        exit(EXIT_FAILURE);
    }

    uint8_t nbytes = size_to_bytes(size); // Used for postincrement and predecrement

    // B, W and L refer to displacements or adresses: 8, 16 or 32 bit, respectively. An refers to an adress register.
    // Dn refers to a data register. Xn referes to either. S refers to the size of the operand in bytes (1, 2 or 4)
    bool mem_access = true;
    uint32_t ea; // Effective address
    uint16_t ext_word;
    BEW bew; // Brief extension word
    switch(M){
        case 0b000: // Dn / <ea> = Dn / Data register direct
            mem_access = false;
            if (write)
                write_Dn(*buf, Xn, size);
            else
                *buf = read_Dn(Xn, size);
            break;
        case 0b001: // An / <ea> = An / Adress register direct
            mem_access = false;
            if (write)
                write_An(*buf, Xn, size);
            else
                *buf = read_An(Xn, size);
            break;
        case 0b010: // (An) / <ea> = [An] / Adress register indirect
            ea = cpu.a[Xn];
            break;
        case 0b011: // (An)+ / <ea> = [An] ; An <- [An] + S / Indirect with postincrement
            // TODO: If register is A7 (stack pointer), it is always kept aligned (never add only 1)
            ea = cpu.a[Xn];
            cpu.a[Xn] += nbytes;
            // TODO: Modify flags for postincrement and predecrement
            break;
        case 0b100: // -(An) / An <- [An] - S ; <ea> = [An] / Indirect with predecrement
            cpu.a[Xn] -= nbytes;
            ea = cpu.a[Xn];
            break;
        case 0b101: // W(An) / <ea> = W + [An] / Indirect with displacement
            // TODO: Test negative displacement
            ea = ((int16_t) fetch_data(WORD)) + cpu.a[Xn];
            break;
        case 0b110: // B(An, Xn.S) / <ea> = B + [An] + [Xn] / Indirect with index
            ext_word = fetch_data(WORD);
            bew = *((BEW*) &ext_word);
            ea = ((int8_t) bew.displacement) + cpu.a[Xn];

            uint8_t s;
            if (bew.lon)
                s = 0b10;
            else
                s = 0b01;

            if (bew.mode) // mode=1 -> An, mode=0 -> Dn
                ea += read_An(bew.n, s);
            else
                ea += read_Dn(bew.n, s);
            break;
        case 0b111:
            switch(Xn) {
                case 0b000: // W / <ea> = [W] / Absolute
                        ea = fetch_data(WORD);
                    break;
                case 0b001: // L / <ea> = [L] / Absolute long
                        ea = fetch_data(LONG);
                    break;
                case 0b010: // W(PC) / <ea> = W + [PC] / relative
                        // TODO: Test negative displacement
                        ea = ((int16_t) fetch_data(WORD)) + cpu.pc;
                    break;
                case 0b011: // B(PC, Xn.S) / <ea> = B + [PC] + [Xn] / relative with offset
                    ext_word = fetch_data(WORD);
                    bew = *((BEW*) &ext_word);
                    ea = ((int8_t) bew.displacement) + cpu.pc;

                    uint8_t s;
                    if (bew.lon)
                        s = 0b10;
                    else
                        s = 0b01;

                    if (bew.mode) // mode=1 -> An, mode=0 -> Dn
                        ea += read_An(bew.n, s);
                    else
                        ea += read_Dn(bew.n, s);
                    break;
                case 0b100: // #k / <ea> = [PC] ; PC <- [PC] + S / Immediate
                        *buf = fetch_data(size);
                    break;
            }
            break;
    }

    if (mem_access) { // If it wasn't a direct register operation, we need to read or write to the ea
        fprintf(stderr, "Effective address calculated: %x\n", ea);
        if (write)
            write_mem(ea, size, *buf);
        else
            *buf = read_mem(ea, size);
    }

    return 0;
}