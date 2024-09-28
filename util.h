#pragma once
    #include<stdio.h>
    #include<stdlib.h>
    #include<stdbool.h>
    #include<stdint.h>
    #include<string.h>

#define INFO 0
#define WARNING 1
#define ERROR -1
void logmsg(int8_t status, char* func, char* message);
uint8_t size_to_bytes(uint8_t size);