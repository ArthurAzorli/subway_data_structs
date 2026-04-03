#include "program.h"
#include "service/database/data_base_repository.h"
#include "service/input/input_repository.h"
#include "core/utils/errors.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_FILE_MAX_LENGTH 101

void Program_requestInput(char *message) {
#ifdef SHOW_INPUT_REQUEST
#if SHOW_INPUT_REQUEST
    if (message != NULL) printf("%s\n", message);
#endif
#endif
}

bool Program_startSession(struct Session *session) {
    if (session == NULL) {
        throwError("Failed to initialize session");
        return false;
    }

    Program_requestInput("Enter the input and output files paths:");
    char inputFilePath[PATH_FILE_MAX_LENGTH], outputFilePath[PATH_FILE_MAX_LENGTH];
    if (scanf("%s %s", inputFilePath, outputFilePath) != 2) {
        throwError("Can not reading paths files");
        return false;
    }

    Session_clear(session);
    session->inputFilePath = strdup(inputFilePath);
    session->outputFilePath = strdup(outputFilePath);


    struct DataBase *dataBase = DataBaseRepository_init(session->outputFilePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    struct InputFile *inputFile = InputRepository_openFile(session->inputFilePath);
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
    return true;
}
