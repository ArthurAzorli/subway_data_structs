#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Prints a 32-bit unsigned integer.
 *
 * If the value is equal to EMPTY, prints "NULO". Otherwise, prints the integer value.
 *
 * @param value The unsigned 32-bit integer to print
 */
void printUint32(const uint32_t value) {
    if (value == EMPTY) {
        printf("NULO");
    } else {
        printf("%d", value);
    }
}

/**
 * @brief Prints a string.
 *
 * If the string is NULL, prints "NULO". Otherwise, prints the string content.
 *
 * @param value Pointer to the string to print
 */
void printString(const char *value) {
    if (value == NULL) {
        printf("NULO");
    } else {
        printf("%s", value);
    }
}

/**
 * @brief Parses a string into a 32-bit unsigned integer.
 *
 * Converts the given string into a uint32_t value. If the string is NULL, empty,
 * or equal to "NULO", returns EMPTY. If the string contains invalid characters,
 * also returns EMPTY.
 *
 * @param value Pointer to the string to parse
 * @return The parsed unsigned integer, or EMPTY if invalid
 */
uint32_t parseUint32(const char *value) {
    if (value == NULL || strcmp(value, "") == 0 || strcmp(value, "NULO") == 0) return EMPTY;
    char *endptr;
    const long converted = strtol(value, &endptr, 10);
    if (*endptr != '\0') return EMPTY;
    return (uint32_t) converted;
}

/**
 * @brief Compares two strings for equality.
 *
 * If the search value is NULL or empty, returns true only if the record value is also NULL.
 * Otherwise, compares both strings using strcmp.
 *
 * @param recordValue The string stored in the record
 * @param value The string to compare against
 * @return true if both strings are equal, false otherwise
 */
bool cmpString(const char *recordValue, const char *value) {
    if (value == NULL || strcmp(value, "") == 0) {
        return recordValue == NULL;
    } else {
        return recordValue != NULL && strcmp(recordValue, value) == 0;
    }
}

/**
 * @brief Reads an integer from input as a string.
 *
 * Reads a value from standard input and stores it as a string. If reading fails,
 * stores an empty string.
 *
 * @param value Pointer to the buffer where the string will be stored
 */
void readIntAsString(char *value) {
    if (scanf("%s", value) != 1) {
        strcpy(value, "");
    }
}