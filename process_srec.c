#include "process_srec.h"
#include <unistd.h>

/* --- LOAD_SREC -----------------------------------------------------------------------------------
 * LOADS S-RECORD FILE'S INSTRUCTIONS INTO RAM AND RETURNS THE ENTRYPOINT ADRESS
*/
uint32_t load_srec(char* filename, CPU* cpu) {
    char* records = read_file(filename); // Obtain string with all the S-records in the file
    uint32_t entryPoint = process_records(records, cpu); // Process them and load into the emulator memory
    free(records); // Now we've already loaded the program into memory, so we can get rid of the S-records
    return entryPoint;
}


/* READ_FILE -----------------------------------------------------------------------------------
 * READS THE SPECIFIED FILE, RESERVES MEMORY FOR IT AND RETURNS POINTER TO IT
*/
char* read_file(char* filename) {
    FILE* file;
    file = fopen(filename, "rb");

    if (file == NULL){
        fprintf(stderr, "Error opening the record file. \"%s\"\n", filename);
        return NULL;
    }

    // Get the file size in bytes
    fseek(file, 0, SEEK_END);
    unsigned int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Reserve memory for the records
    char* buf;
    buf = (char*) malloc(filesize);

    // Read the entire file into memory
    fread(buf, 1, filesize, file);

    fclose(file);

    return buf;
}


/* --- PROCESS_RECORDS -----------------------------------------------------------------------------------
 * Processes S-records, skipping all but S3 and S7 records. For those records, writeBytes is called to read
 * the adress and data (only for S3) bytes, and act accordingly. S7 records indicates the end of program, and
 * does not contain data bytes, only adress bytes with the entrypoint adress. Returns the entrypoing adress.
 * S3 RECORD STRUCTURE: S3LLAAAAAAAADD...DDCC             S7 RECORD STRUCTURE: S7LLAAAAAAAACC
 * Where:
 *      -> "S3" and "S7" are those literal characters, indicating the record type. All remaining characters are hex characters that encode bytes in pairs.
 *      -> The LL byte indicates the amount of remaining characters (excluding newline)
 *      -> AAAAAAAA encodes 4 bytes corresponding to the start adress of the record
 *      -> DD...DD is a variable number of data bytes, which are to be written starting from the afromentined start adress
 *      -> CC is a checksum byte which can be ignored
*/
uint32_t process_records(char* rec, CPU* cpu) {
    char* pos = rec;  // Position in the s-record file

    while(pos[1] != '7') { // Process each record until we reach the termination record
        int cant = 2*parseHex(pos+2) + 1;  // Read character 3 and 4, which are hex for a number that indicates the number of bytes remaining

        if (pos[1] == '3') {  // If it is a data record (S3)
            writeBytes(pos+4, cant-3, cpu); // substract 2 checksum characters + 1 newline chracter
        }

        pos += 4 + cant;
    }

    // Process last record (obtain entrypoint)
    uint32_t ep = writeBytes(pos+4, 8, cpu);
    printf("ENTRY POINT ADDRESS: %x\n", ep);
    return ep;
}

/* --- PARSEHEX -----------------------------------------------------------------------------------
 * Reads 2 hex characters starting at 'str'. Parses them as a byte and returns the value.
*/
uint8_t parseHex(char* str) {
    uint8_t byte = 0;

    // Runs twice: once with val=16 and another time with val=1
    // Advances 1 character to the right at the end of iteration
    for (char val = 16; val > 0; val -= 15, str++) {
        if (*str - '0' < 10) // The digit is 0-9
            byte += (*str-'0')*val;
        else // The digit is A-F
            byte += (10 + (*str - 'A'))*val;
    }

    return byte;
}

/* --- WRITEBYTES -----------------------------------------------------------------------------------
 * Reads adress and data (if applicable) bytes from the record, starting at 'start'. Writes each data
 * byte starting from the record address until the end of the data (indicated by 'len'). Returns the
 * record address. For final S7 record, no data is written.
*/
uint32_t writeBytes(char* start, int len, CPU* cpu) {
    uint32_t address = 0; // Read adress bytes (4 bytes, 8 hex chars)
    address += parseHex(start)*16777216;
    start += 2;
    address += parseHex(start)*65536;
    start += 2;
    address += parseHex(start)*256;
    start += 2;
    address += parseHex(start);
    start += 2; // Start now points to the first data character

    fprintf(stderr, "Writing %d bytes to adress 0x%x\n", (len-8)/2, address);

    // Write data contained in the S-RECORD to the specified address
    int dataChars = len-8;
    for (int offset = 0; offset < dataChars; offset += 2) {
        cpu->ram[address+(offset/2)] = parseHex(start+offset);
        fprintf(stderr, "Wrote byte %x to address %x\n", parseHex(start+offset), address+(offset/2));
    }
    fprintf(stderr, "--------------------------------------------------\n");

    return address;
}