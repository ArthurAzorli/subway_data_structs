/**
 * @file errors.h
 * @brief Error handling utilities for the application.
 */

#ifndef TRABALHO01_ERRORS_H
#define TRABALHO01_ERRORS_H

#include <stdio.h>

#include "../../config.h"

/**
 * @brief Outputs an error message to stderr based on configuration settings.
 *
 * This function prints error messages conditionally based on the SHOW_ERRORS
 * and SIMPLE_ERRORS configuration macros. If SIMPLE_ERRORS is enabled, a
 * generic error message is shown; otherwise, the provided message is displayed.
 *
 * @param message: The error message to display
 */
static inline void throwError(const char *message) {
#ifdef SHOW_ERRORS
#if SHOW_ERRORS
#ifdef SIMPLE_ERRORS
#if SIMPLE_ERRORS
    fprintf(stderr, "Falha no processamento do arquivo.\n");
#else
    fprintf(stderr, "ERROR: %s\n", message);
#endif
#endif
#endif
#endif
}

#endif //TRABALHO01_ERRORS_H