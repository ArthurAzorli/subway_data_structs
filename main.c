/**
 * SCC0503 - Algoritmos e Estruturas de Dados II
 * Trabalho Prático 1 - Sistema de Arquivos Binários de Metrô/CPTM
 * ICMC - Instituto de Ciências Matemáticas e de Computação - USP São Carlos
 *
 * Repositório Github: https://github.com/ArthurAzorli/subway_data_structs;
 *
 * GRUPO 24
 * @author Arthur Gagliardi Azorli - 16855452
 * @author Nícolas Silva Scorza - 17025079
 */

#include <stdio.h>
#include "project/tests/file_repository_test.c"
#include "project/tests/header_repository_test.c"
#include "project/tests/record_repository_test.c"
#include "project/tests/input_repository_test.c"
#include "project/tests/string_test.c"
#include "project/program.h"


int main() {
    file_repository_test();
    header_repository_test();
    record_repository_test();
    input_repository_test();
    string_test();
    int option;
    if (scanf("%d", &option) != 1) return 1;

    switch (option) {
        case 1: {
            if (!Program_initSubwayFile()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 2: {
            if (!Program_showRecords()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 3: {
            if (!Program_searchRecord()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 4: {
            if (!Program_getRecordByRRN()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        default:


    }
    return 0;
}
