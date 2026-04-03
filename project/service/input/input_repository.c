#include "input_repository.h"

#include  "../../core/utils/errors.h"
#include <stdlib.h>

#include "../../core/utils/string.h"

#define LINE_MAX_LENGTH 1024
#define FIELD_DELIMITER ","

struct InputFile {
    FILE *file;
    size_t line;
};

bool InputRepository_isInputFileValid(const struct InputFile *inputFile) {
    return inputFile != NULL && inputFile->file != NULL;
}

uint32_t InputRepository_parseUInt(const String input) {
    if (input == NULL || strlen(input) == 0) return EMPTY;
    char *endPtr;
    const unsigned long value = strtoul(input, &endPtr, 10);
    if (*endPtr != '\0') {
        throwError("Failed to parse unsigned integer");
        return EMPTY;
    }
    return (uint32_t) value;
}

struct InputFile *InputRepository_openFile(const String path) {
    if (path == NULL) {
        throwError("Invalid file path");
        return NULL;
    }
    struct InputFile *inputFile = malloc(sizeof(struct InputFile));
    if (inputFile == NULL) {
        throwError("Failed to allocate input file");
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        throwError("Failed to open file for reading");
        free(inputFile);
        return NULL;
    }
    inputFile->file = file;
    inputFile->line = 0;
    return inputFile;
}

struct SubwayRecord *InputRepository_extractRecord(struct InputFile *inputFile) {
    if (!InputRepository_isInputFileValid(inputFile)) {
        throwError("Input file is invalid");
        return NULL;
    }

    char line[LINE_MAX_LENGTH];

    //ignore the first line of definitions
    if (inputFile->line == 0) {
        fgets(line, LINE_MAX_LENGTH, inputFile->file);
        inputFile->line++;
    }

    if (fgets(line, LINE_MAX_LENGTH, inputFile->file) == NULL) return NULL;
    line[strcspn(line, "\r\n")] = 0;
    inputFile->line++;

    char *stationID, *stationName, *lineID, *lineName, *destinationID, *distant, *interactionLineID, *
            interactionStationID;
    split(line, FIELD_DELIMITER, &stationID, &stationName, &lineID, &lineName, &destinationID, &distant,
          &interactionLineID, &interactionStationID);

    struct SubwayRecord *record = SubwayRecord_init();
    if (record == NULL) {
        throwError("Failed to allocate subway record");
        return NULL;
    }

    record->originStationID = InputRepository_parseUInt(stationID);
    record->originLineID = InputRepository_parseUInt(lineID);
    record->destinationStationID = InputRepository_parseUInt(destinationID);
    record->destinationDistant = InputRepository_parseUInt(distant);
    record->interactionLineID = InputRepository_parseUInt(interactionLineID);
    record->interactionStationID = InputRepository_parseUInt(interactionStationID);

    if (stationName != NULL) {
        record->stationNameLength = strlen(stationName);
        record->stationName = strdup(stationName);
    } else {
        record->stationNameLength = 0;
        record->stationName = NULL;
    }

    if (lineName != NULL) {
        record->lineNameLength = strlen(lineName);
        record->lineName = strdup(lineName);
    } else {
        record->lineNameLength = 0;
        record->lineName = NULL;
    }

    free(stationID);
    free(lineID);
    free(destinationID);
    free(interactionLineID);
    free(interactionStationID);
    free(stationName);
    free(lineName);

    return record;
}

void InputRepository_closeFile(struct InputFile *inputFile) {
    if (!InputRepository_isInputFileValid(inputFile)) return;
    fclose(inputFile->file);
    inputFile->file = NULL;
    free(inputFile);
}
