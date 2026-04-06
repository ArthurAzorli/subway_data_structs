/**
 * @file types.h
 * @brief Defines custom types and constants used throughout the project.
 */

#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>

/** @brief Special value representing an empty or uninitialized state. */
#define EMPTY 0xFFFFFFFF

#define true 1
#define false 0

/** @brief Custom boolean type using unsigned 8-bit integer. */
#ifndef bool
  #define bool uint8_t
#endif
typedef const char *String;

#endif //TYPES_H
