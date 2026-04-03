#include <stdio.h>

#include "project/program.h"
#include "project/config.h"
#include "project/lib/provided.h"
#include "project/core/utils/errors.h"

#ifdef RUN_TESTS
#if RUN_TESTS
#include "project/tests/string_test.c"
#include "project/tests/file_repository_test.c"
#include "project/tests/header_repository_test.c"
#include "project/tests/record_repository_test.c"
#include "project/tests/data_base_repository_test.c"
#include "project/tests/input_repository_test.c"
#endif
#endif


void runTests() {
#ifdef RUN_TESTS
#if RUN_TESTS
    printf("Starting tests...\n");
    string_test();
    file_repository_test();
    record_repository_test();
    header_repository_test();
    database_repository_test();
    input_repository_test();
    printf("Finish tests\n");
#endif
#endif
}

void showMenu() {
#ifdef SHOW_MENU
#if SHOW_MENU
    printf("\n");
    printf("=============================\n");
    printf("             Menu            \n");
    printf("=============================\n");
    printf("\n");
    printf("1. Start Session\n");
    printf("0. Exit\n");
    printf("\n");
    printf("Enter a option:\n");
#endif
#endif
}

int main() {
    runTests();
    int option;
    do {
        showMenu();
        if (scanf("%d", &option) != 1) {
            throwError("Incorrect optional input");
            continue;
        }
        switch (option) {
            case 1: {
                if (!Program_readFromFile()) {
                    throwError("Could not start session");
                    return 1;
                }
                break;
            }
            case 2: {
                if (!Program_showRecords()) {
                    throwError("Could not start session");
                    return 1;
                }
                break;
            }
            case 3: {
                if (!Program_searchRecord()) {
                    throwError("Could not start session");
                    return 1;
                }
                break;
            }
            case 4: {
                if (!Program_getRecordByRRN()) {
                    throwError("Could not start session");
                    return 1;
                }
                break;
            }
            case 0:
                continue;
            default:
                throwError("Unrecognized option");
        }

    } while (option != 0);
    return 0;
}


void function2 () {
    if (scanf("%s", entrada) != 1) {
        return 0;
    }
    FILE *binary = fopen(entrada, "rb");
    if (binary == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }
    fclose(binary);
    dataBase = DataBaseRepository_init(entrada);
    if (dataBase == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    size_t const totalSize = FileRepository_fileSize(dataBase->dataFile);

    if (totalSize < 16) {
        printf("Registro inexistente.\n");
        DataBaseRepository_close(dataBase);
        break;
    }

    size_t const totalRecord = (totalSize - 16) / 80;

    bool printedRecord = false;

    for (int i = 0; i < totalRecord; i++) {
        struct SubwayRecord *rec = DataBaseRepository_readRecord(dataBase, i);
        if (rec != NULL) {
            printRecord(rec);
            printedRecord = true;
            SubwayRecord_free(rec);
        }
    }

    if (!printedRecord) {
        printf("Registro inexistente.\n");
    }

    DataBaseRepository_close(dataBase);
}

void function4() {
    size_t rrn;
    if (scanf("%s %zu", entrada, &rrn) != 2) {
        return 0;
    }

    FILE *binary = fopen(entrada, "rb");

    if (binary == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }
    fclose(binary);

    dataBase = DataBaseRepository_init(entrada);
    if (dataBase == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    struct SubwayRecord *rec = DataBaseRepository_readRecord(dataBase, rrn);
    if (rec != NULL) {
        printRecord(rec);
        SubwayRecord_free(rec);
    } else {
        printf("Registro inexistente.\n");
    }
    DataBaseRepository_close(dataBase);
    break;
}

void printRecord(struct SubwayRecord *rec) {
    if (rec->originStationID != EMPTY) printf("%d ", rec->originStationID);
    else printf("NULO ");
    if (rec->stationName != NULL) printf("%s ", rec->stationName);
    else printf("NULO ");
    if (rec->originLineID != EMPTY) printf("%d ", rec->originLineID);
    else printf("NULO ");
    if (rec->lineName != NULL) printf("%s ", rec->lineName);
    else printf("NULO ");
    if (rec->destinationStationID != EMPTY) printf("%d ", rec->destinationStationID);
    else printf("NULO ");
    if (rec->destinationDistant != EMPTY) printf("%d ", rec->destinationDistant);
    else printf("NULO ");
    if (rec->interactionLineID != EMPTY) printf("%d ", rec->interactionLineID);
    else printf("NULO ");
    if (rec->interactionStationID != EMPTY) printf("%d", rec->interactionStationID);
    else printf("NULO");
    printf("\n");
}