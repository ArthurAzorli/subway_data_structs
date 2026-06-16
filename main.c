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
#include "project/program.h"


int main() {
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
        case 5: {
            if (!Program_initIndexableFile()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 6: {
            if (!Program_searchRecordByIndexable()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 7: {
            if (!Program_removeRecords()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 8: {
            if (!Program_insertRecord()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        case 9: {
            if (!Program_updateRecord()) printf("Falha no processamento do arquivo.\n");
            break;
        }
        default: {
            return 1;
        }
    }
    return 0;
}
