#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifndef EMPTY
#define EMPTY 0xFFFFFFFF
#endif

typedef const char *String;

void printUint32(const uint32_t value);

void printString(const char *value);

uint32_t parseUint32(const char *value);

bool cmpString(const char *recordValue, const char *value);

void readIntAsString(char *value);

#endif
