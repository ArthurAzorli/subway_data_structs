#include "program.h"
#include "service/database/data_base_repository.h"
#include "service/input/input_repository.h"
#include "core/utils/errors.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "lib/provided.h"
#include "service/database/header_repository.h"

#define INPUT_MAX_LENGTH 101

void Program_requestInput(char *message) {
#ifdef SHOW_INPUT_REQUEST
#if SHOW_INPUT_REQUEST
    if (message != NULL) printf("%s\n", message);
#endif
#endif
}

void Program_printRecord(const struct SubwayRecord *record) {
    if (record->originStationID != EMPTY) printf("%d ", record->originStationID);
    else printf("NULO ");
    if (record->stationName != NULL) printf("%s ", record->stationName);
    else printf("NULO ");
    if (record->originLineID != EMPTY) printf("%d ", record->originLineID);
    else printf("NULO ");
    if (record->lineName != NULL) printf("%s ", record->lineName);
    else printf("NULO ");
    if (record->destinationStationID != EMPTY) printf("%d ", record->destinationStationID);
    else printf("NULO ");
    if (record->destinationDistant != EMPTY) printf("%d ", record->destinationDistant);
    else printf("NULO ");
    if (record->interactionLineID != EMPTY) printf("%d ", record->interactionLineID);
    else printf("NULO ");
    if (record->interactionStationID != EMPTY) printf("%d", record->interactionStationID);
    else printf("NULO");
    printf("\n");
}

bool Program_readFromFile() {
    Program_requestInput("Enter the input and output files paths:");
    char inputFilePath[INPUT_MAX_LENGTH], outputFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s", inputFilePath, outputFilePath) != 2) {
        throwError("Can not reading paths files");
        return false;
    }

    struct DataBase *dataBase = DataBaseRepository_init(outputFilePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    struct InputFile *inputFile = InputRepository_openFile(inputFilePath);
    if (inputFile == NULL) {
        throwError("Failed to open file");
        return false;
    }

    struct SubwayRecord *record;
    while ((record = InputRepository_extractRecord(inputFile)) != NULL) {
        if (!DataBaseRepository_createRecord(dataBase, record)) {
            throwError("Failed to create record in data base");
            SubwayRecord_free(record);
            return false;
        }
        SubwayRecord_free(record);
    }

    InputRepository_closeFile(inputFile);
    DataBaseRepository_close(dataBase);
    BinarioNaTela(outputFilePath);
    return true;
}

bool Program_showRecords() {
    Program_requestInput("Enter the file path:");
    char filePath[INPUT_MAX_LENGTH];
    if (scanf("%s", filePath) != 1) {
        throwError("Failed read file path");
        return false;
    }

    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    bool printedRecord = false;
    for (int i = 0; i < dataBase->dataHeader->nextInsert; i++) {
        struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, i);
        if (record == NULL) continue;
        Program_printRecord(record);
        printedRecord = true;
        SubwayRecord_free(record);
    }

    if (!printedRecord) printf("Registro inexistente.\n");
    DataBaseRepository_close(dataBase);
    return true;
}

bool Program_searchRecord() {
    return true;
}

bool Program_getRecordByRRN() {
    uint32_t rrn;
    char filePath[INPUT_MAX_LENGTH];
    Program_requestInput("Enter the file path and RRN:");
    if (scanf("%s %u", filePath, &rrn) != 2) {
        throwError("Failed read file path and RRN");
        return false;
    }

    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, rrn);
    if (record == NULL) {
        printf("Registro inexistente.\n");
    } else {
        Program_printRecord(record);
        SubwayRecord_free(record);
    }

    DataBaseRepository_close(dataBase);
    return true;
}