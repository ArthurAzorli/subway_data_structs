/**
 * @file string.h
 * @brief String manipulation utilities for parsing and splitting strings.
 */

#ifndef TRABALHO01_STRING_H
#define TRABALHO01_STRING_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal implementation function for splitting strings by a delimiter token.
 *
 * This function splits an input string by a delimiter token and stores the resulting
 * fields in the provided array of pointers.
 *
 * @param input: The input string to split
 * @param token: The delimiter string to split on
 * @param fields: Pointer to array of field pointers
 * @param fieldsCount: Maximum number of fields to extract
 *
 * @return The number of fields actually extracted and stored
 */
static size_t _split_impl(const char *input, const char *token, char **fields[], const size_t fieldsCount) {
    size_t count = 0;
    const char *start = input;
    const size_t tokenLength = strlen(token);

    while (count < fieldsCount) {
        const char *index = strstr(start, token);
        const size_t length = index ? (size_t)(index - start) : strlen(start);

        if (length == 0) {
            *fields[count] = NULL;
        } else {
            *fields[count] = malloc(length + 1);
            memcpy(*fields[count], start, length);
            (*fields[count])[length] = '\0';
        }

        count++;
        if (!index) break;
        start = index + tokenLength;
    }
    return count;
}

/**
 * @brief Macro for splitting a string by a delimiter token into multiple fields.
 *
 * This macro provides a convenient interface to split a string and automatically
 * populate multiple variables with the resulting fields.
 *
 * @param input: The input string to split
 * @param token: The delimiter string
 * @param ...: Variable number of char* pointers to receive the split fields
 *
 * @example
 *   char *field1, *field2, *field3;
 *   split("hello,world,test", ",", &field1, &field2, &field3);
 *
 * @note Caller must free each allocated field after use.
 */
#define split(input, token, ...) \
_split_impl((input), (token), (char**[]){__VA_ARGS__}, sizeof((char**[]){__VA_ARGS__})/sizeof(char**))

#endif //TRABALHO01_STRING_H
