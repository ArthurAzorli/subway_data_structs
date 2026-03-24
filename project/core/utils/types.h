#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>

#define EMPTY 0xFFFFFFFF

#define true 1
#define false 0

typedef uint16_t bool;
typedef const char* String;

struct DateTime {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

#endif //TYPES_H
