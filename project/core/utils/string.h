#ifndef TRABALHO01_STRING_H
#define TRABALHO01_STRING_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

#define split(input, token, ...) \
_split_impl((input), (token), (char**[]){__VA_ARGS__}, sizeof((char**[]){__VA_ARGS__})/sizeof(char**))

#endif //TRABALHO01_STRING_H
