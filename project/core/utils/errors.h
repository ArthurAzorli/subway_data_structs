/**
 * @file errors.h
 * @brief Error handling utilities for the application.
 */

#ifndef TRABALHO01_ERRORS_H
#define TRABALHO01_ERRORS_H

#include <stdio.h>
#include "../../config.h"

// Flag global para controlar se já foi exibida a mensagem simples
static int simple_error_printed = 0;

/**
 * @brief Outputs an error message to stderr based on configuration settings.
 *
 * This function prints error messages conditionally based on the SHOW_ERRORS
 * and SIMPLE_ERRORS configuration macros. If SIMPLE_ERRORS is enabled, a
 * generic error message is shown; otherwise, the provided message is displayed.
 * If SIMPLE_ERRORS is enabled, a generic error message is shown only once
 * during the entire execution. Otherwise, the provided message is displayed
 * every time.
 *
 * @param message: The error message to display
 */
static inline void throwError(const char *message) {
#if defined(SHOW_ERRORS) && SHOW_ERRORS
    fprintf(stderr, "ERROR: %s\n", message);
#endif
}

#endif // TRABALHO01_ERRORS_H
