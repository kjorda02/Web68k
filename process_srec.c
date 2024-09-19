#include "process_srec.h"
#include <unistd.h>
uint32_t entryPoint;

int main(){
    initCpu();
    unsigned int filesize;
    char* records = read_file(&filesize);
    process_records(records);
    // printf("Numero: %d\n", parseHex("FA"));
}


char* read_file(unsigned int* size) {
    FILE* file;
    file = fopen("output", "rb");

    if (file == NULL){
        fprintf(stderr, "Error opening the record file.\n");
        return NULL;
    }

    // Get the file size in bytes
    fseek(file, 0, SEEK_END);
    unsigned int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Reserve memory for the records
    char* records = (char*) malloc(filesize);

    // Read the entire file into memory
    fread(records, 1, filesize, file);

    fclose(file);

    *size = filesize;
    return records;
}


/* PROCESSES S-RECORDS AND WRITES THEIR DATA TO THE ADRESS SPECIFIED IN THE RECORD
 * S3 RECORDS CONTAIN DATA. S7 RECORDS END THE FILE AND INDICATE THE ENTRY POINT.
 * ALL OTHER RECORDS ARE IGNORED.
*/
void process_records(char* rec) {
    char* pos = rec;  // Position in the s-record file

    while(pos[1] != '7') { // Process each record until we reach the termination record
        int cant = 2*parseHex(pos+2) + 1;  // Read character 3 and 4, which are hex for a number that indicates the number of bytes remaining

        if (pos[1] == '3') {  // If it is a data record (S3)
            writeBytes(pos+4, cant-2); // Last 2 characters are checksum, not data
        }

        pos += 4 + cant;
    }

    // Process last record (obtain entrypoint)
    entryPoint = writeBytes(pos+4, 8);
    printf("ENTRY POINT ADDRESS: %x\n", entryPoint);
}

// Gets byte from 2 hex characters
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


uint32_t writeBytes(char* start, int len) {
    uint32_t address = 0; // Read adress bytes (4 bytes, 8 hex chars)
    address += parseHex(start)*16777216;
    start += 2;
    address += parseHex(start)*65536;
    start += 2;
    address += parseHex(start)*256;
    start += 2;
    address += parseHex(start);
    start += 2; // Start now points to the first data character

    printf("Writing %d bytes to adress 0x%x\n", (len-8)/2, address);

    // Write data contained in the S-RECORD to the specified address
    int dataChars = len-8;
    for (int offset = 0; offset < dataChars; offset += 2) {
        //cpu.ram[address+(offset/2)] = parseHex(start+offset);
        printf("Wrote byte %x to address %x\n", parseHex(start+offset), address+(offset/2));
    }
    printf("--------------------------------------------------\n");
    return address;
}