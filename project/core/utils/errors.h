#ifndef TRABALHO01_ERRORS_H
#define TRABALHO01_ERRORS_H

#include <stdio.h>

#include "../../config.h"

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