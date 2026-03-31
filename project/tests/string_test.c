#include <assert.h>
#include "../core/utils/string.h"

#include <stdio.h>

void string_test() {
    // String vazia
    char *c1, *c2;
    size_t n = split("", ",", &c1, &c2);
    assert(n == 1);
    assert(c1 == NULL);

    // Delimitadores consecutivos
    char *d1, *d2, *d3;
    n = split("A,,B", ",", &d1, &d2, &d3);
    assert(n == 3);
    assert(strcmp(d1, "A") == 0);
    assert(d2 == NULL);
    assert(strcmp(d3, "B") == 0);
    free(d1);
    free(d3);

    // Delimitador no início
    char *e1, *e2;
    n = split(",X", ",", &e1, &e2);
    assert(n == 2);
    assert(e1 == NULL);
    assert(strcmp(e2, "X") == 0);
    free(e2);

    // Delimitador no fim
    char *f1, *f2;
    n = split("Y,", ",", &f1, &f2);
    assert(n == 2);
    assert(strcmp(f1, "Y") == 0);
    assert(f2 == NULL);
    free(f1);

    // Delimitador no meio
    char *g1, *g2, *g3;
    n = split("A,B,C", ",", &g1, &g2, &g3);
    assert(n == 3);
    assert(strcmp(g1, "A") == 0);
    assert(strcmp(g2, "B") == 0);
    assert(strcmp(g3, "C") == 0);
    free(g1);
    free(g2);
    free(g3);

    // Delimitador múltiplo (", ")
    char *h1, *h2, *h3;
    n = split("A, B, C", ", ", &h1, &h2, &h3);
    assert(n == 3);
    assert(strcmp(h1, "A") == 0);
    assert(strcmp(h2, "B") == 0);
    assert(strcmp(h3, "C") == 0);
    free(h1);
    free(h2);
    free(h3);

    printf("STRING: OK\n");
}
