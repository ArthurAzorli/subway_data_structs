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

/**
 * @brief Prints an input request message to the user.
 *
 * Displays a message prompting for user input, controlled by
 * SHOW_INPUT_REQUEST configuration option.
 *
 * @param message: The prompt message to display (can be NULL)
 */
void Program_requestInput(char *message) {
#ifdef SHOW_INPUT_REQUEST
#if SHOW_INPUT_REQUEST
    if (message != NULL) printf("%s\n", message);
#endif
#endif
}

/**
 * @brief Displays a subway record in human-readable format.
 *
 * Prints all fields of a record, using "NULO" for empty/null values.
 * Fields are space-separated on a single line.
 *
 * @param record: The record to display (must not be NULL)
 */
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

/**
 * @brief Reads subway records from a file and imports them into the database.
 *
 * Prompts the user for input and output file paths, opens the input file,
 * parses each record, and inserts them into the binary database file.
 *
 * @return true if import completed successfully, false on error
 */
bool Program_readFromFile() {
    //le caminho dos arquivos
    Program_requestInput("Enter the input and output files paths:");
    char inputFilePath[INPUT_MAX_LENGTH], outputFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s", inputFilePath, outputFilePath) != 2) {
        throwError("Can not reading paths files");
        return false;
    }

    //inicializa database
    struct DataBase *dataBase = DataBaseRepository_init(outputFilePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    //inicializa input file
    struct InputFile *inputFile = InputRepository_openFile(inputFilePath);
    if (inputFile == NULL) {
        throwError("Failed to open file");
        return false;
    }

    //extrai os registros do arquivo de entrada e salva no banco de dados enquanto houver registros
    struct SubwayRecord *record;
    while ((record = InputRepository_extractRecord(inputFile)) != NULL) {
        if (!DataBaseRepository_createRecord(dataBase, record)) {
            throwError("Failed to create record in data base");
            SubwayRecord_free(record);
            return false;
        }
        SubwayRecord_free(record);
    }

    //finaliza procedimento
    InputRepository_closeFile(inputFile);
    DataBaseRepository_close(dataBase);
    BinarioNaTela(outputFilePath);
    return true;
}

/**
 * @brief Displays all active (non-deleted) records in the database.
 * @return true if display completed successfully, false on error
 */
bool Program_showRecords() {
    //le caminho do aqruivo binário
    Program_requestInput("Enter the file path:");
    char filePath[INPUT_MAX_LENGTH];
    if (scanf("%s", filePath) != 1) {
        throwError("Failed read file path");
        return false;
    }

    //iniciliza database
    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    //printa cada registro do data base
    bool printedRecord = false;
    for (int i = 0; i < dataBase->dataHeader->nextInsert; i++) {
        struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, i);
        if (record == NULL) continue;
        Program_printRecord(record);
        printedRecord = true;
        SubwayRecord_free(record);
    }

    //se nao ha registros printa mensagem
    if (!printedRecord) printf("Registro inexistente.\n");

    //finliza procedimento
    DataBaseRepository_close(dataBase);
    return true;
}

bool Program_searchRecord() {
    return true;
}

/**
 * @brief Retrieves and displays a specific record by its RRN.
 * @return true if record was found and displayed, false if RRN invalid or error
 */
bool Program_getRecordByRRN() {
    uint32_t rrn;
    char filePath[INPUT_MAX_LENGTH];
    // le caminho do arquivo binario e o rrn do regitrso desejado
    Program_requestInput("Enter the file path and RRN:");
    if (scanf("%s %u", filePath, &rrn) != 2) {
        throwError("Failed read file path and RRN");
        return false;
    }

    //inicializa database
    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    //le o regitro se conseguiu o printa se nao fala que registro nao existe
    struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, rrn);
    if (record == NULL) {
        printf("Registro inexistente.\n");
    } else {
        Program_printRecord(record);
        SubwayRecord_free(record);
    }

    //finaliza procedimento
    DataBaseRepository_close(dataBase);
    return true;
}