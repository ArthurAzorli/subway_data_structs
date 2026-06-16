#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

/** @brief Special value representing an empty or uninitialized state. */
#ifndef EMPTY
#define EMPTY 0xFFFFFFFF
#endif

typedef const char *String;

/**
 * @brief Prints an unsigned 32-bit integer value.
 *
 * Displays the numeric value if it is not EMPTY. If the value is equal to EMPTY,
 * prints "NULO" instead. Used to standardize the output of integer fields in records.
 *
 * @param value: The unsigned 32-bit integer to print
 */
void printUint32(const uint32_t value);

/**
 * @brief Prints a string value.
 *
 * Displays the string if it is not NULL. If the string pointer is NULL,
 * prints "NULO" instead. Used to standardize the output of string fields in records.
 *
 * @param value: The string to print (can be NULL)
 */
void printString(const char *value);

uint32_t parseUint32(const char *value);

/**
 * @brief Compares a string field value with a search string.
 *
 * Treats empty string as NULL. Returns true if both are NULL or if the record string
 * matches the search string exactly.
 *
 * @param recordValue: The string value from the record field to compare
 * @param value: The string value to compare against
 * @return true if values match, false otherwise
 */
bool cmpString(const char *recordValue, const char *value);

/**
 * @brief Reads an integer from stdin and stores it as a string.
 *
 * Attempts to capture the next whitespace-delimited input from the user
 * and place it into the provided buffer. This allows numeric values or
 * the keyword "NULO" to be handled uniformly as strings for later parsing.
 * If the read fails, the buffer is set to an empty string to avoid
 * undefined content.
 *
 * @param value: Pointer to the character buffer where the input will be stored
 */
void readIntAsString(char *value);

#endif
