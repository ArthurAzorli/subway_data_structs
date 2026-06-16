#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printUint32(const uint32_t value) {
    if (value == EMPTY) {
        printf("NULO");
    } else {
        printf("%d", value);
    }
}

void printString(const char *value) {
    if (value == NULL) {
        printf("NULO");
    } else {
        printf("%s", value);
    }
}

uint32_t parseUint32(const char *value) {
    if (value == NULL || strcmp(value, "") == 0 || strcmp(value, "NULO") == 0) return EMPTY;
    char *endptr;
    const long converted = strtol(value, &endptr, 10);
    if (*endptr != '\0') return EMPTY;
    return (uint32_t) converted;
}

bool cmpString(const char *recordValue, const char *value) {
    if (value == NULL || strcmp(value, "") == 0) {
        return recordValue == NULL;
    } else {
        return recordValue != NULL && strcmp(recordValue, value) == 0;
    }
}

void readIntAsString(char *value) {
    if (scanf("%s", value) != 1) {
        strcpy(value, "");
    }
}