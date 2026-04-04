#include "input_repository.h"

#include  "../../core/utils/errors.h"
#include <stdlib.h>

#include "../../core/utils/string.h"

#define LINE_MAX_LENGTH 1024
#define FIELD_DELIMITER ","

/**
 * @struct InputFile
 * @brief Represents an open input file.
 *
 * Maintains file pointer and tracks current line number for parsing.
 */
struct InputFile {
    FILE *file;      /**< Pointer to the open file */
    size_t line;     /**< Current line number  */
};

/**
 * @brief Validates that an InputFile structure is properly initialized.
 * @param inputFile: The input file to check
 * @return true if file is valid and open, false otherwise
 */
bool InputRepository_isInputFileValid(const struct InputFile *inputFile) {
    return inputFile != NULL && inputFile->file != NULL;
}

/**
 * @brief Parses a string to an unsigned 32-bit integer.
 *
 * Converts a numeric string to uint32_t. Returns EMPTY if the string is empty
 * or contains non-numeric characters.
 *
 * @param input: String to parse (can be NULL)
 * @return Parsed integer value, or EMPTY if parsing fails or string is empty
 */
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

/**
 * @brief Opens a CSV file for reading records.
 *
 * Opens a text file containing comma-separated subway station data in read mode.
 * The first line (header) is automatically skipped during first extraction.
 *
 * @param path: Path to the CSV file
 * @return Pointer to allocated InputFile structure on success, NULL on failure
 *
 * @note The returned pointer must be freed using InputRepository_closeFile()
 */
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
    //abre o arquivo apenas paera leitura
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        throwError("Failed to open file for reading");
        free(inputFile);
        return NULL;
    }
    //inicializa os campos
    inputFile->file = file;
    inputFile->line = 0;
    return inputFile;
}

/**
 * @brief Extracts the next record from the input file.
 *
 * Reads the next line from the input file, parses all comma-separated fields,
 * converts them to appropriate types, and returns a SubwayRecord structure.
 * The first call automatically skips the header line.
 *
 * @param inputFile: Open input file to read from
 * @return Pointer to allocated SubwayRecord on success, NULL when EOF or on error
 * @note Returns NULL at EOF
 */
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

    //pega a linha atual removendo o bytes de fim de arquivo
    if (fgets(line, LINE_MAX_LENGTH, inputFile->file) == NULL) return NULL;
    line[strcspn(line, "\r\n")] = 0;
    inputFile->line++;

    //pega os campos a partir da linha lida
    char *stationID, *stationName, *lineID, *lineName, *destinationID, *distant, *interactionLineID, *
            interactionStationID;
    split(line, FIELD_DELIMITER, &stationID, &stationName, &lineID, &lineName, &destinationID, &distant,
          &interactionLineID, &interactionStationID);

    //preenche o registro com os campos lidos
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

/**
 * @brief Closes the input file and frees resources.
 * @param inputFile: The input file to close (can be NULL)
 */
void InputRepository_closeFile(struct InputFile *inputFile) {
    if (!InputRepository_isInputFileValid(inputFile)) return;
    fclose(inputFile->file);
    inputFile->file = NULL;
    free(inputFile);
}
