#include "program.h"
#include "service/database/data_base_repository.h"
#include "service/input/input_repository.h"
#include "core/utils/errors.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "lib/provided.h"
#include "service/database/header_repository.h"
#include  "domain/search_criteria.h"

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
 * @brief Prints an unsigned 32-bit integer value.
 *
 * Displays the numeric value if it is not EMPTY. If the value is equal to EMPTY,
 * prints "NULO" instead. Used to standardize the output of integer fields in records.
 *
 * @param value: The unsigned 32-bit integer to print
 */
void Program_printUint32(const uint32_t value) {
    if (value == EMPTY) {
        printf("NULO");
    } else {
        printf("%d", value);
    }
}


/**
 * @brief Prints a string value.
 *
 * Displays the string if it is not NULL. If the string pointer is NULL,
 * prints "NULO" instead. Used to standardize the output of string fields in records.
 *
 * @param value: The string to print (can be NULL)
 */
void Program_printString(const char *value) {
    if (value == NULL) {
        printf("NULO");
    } else {
        printf("%s", value);
    }
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
    Program_printUint32(record->originStationID);
    printf(" ");
    Program_printString(record->stationName);
    printf(" ");
    Program_printUint32(record->originLineID);
    printf(" ");
    Program_printString(record->lineName);
    printf(" ");
    Program_printUint32(record->destinationStationID);
    printf(" ");
    Program_printUint32(record->destinationDistant);
    printf(" ");
    Program_printUint32(record->interactionLineID);
    printf(" ");
    Program_printUint32(record->interactionStationID);
    printf("\n");
}


/**
 * @brief Compares an unsigned 32-bit integer field value with a string representation.
 *
 * Converts the string value to an unsigned integer (treating empty string as EMPTY constant)
 * and compares it with the record's field value. Used for matching numeric fields during search.
 *
 * @param recordValue: The numeric value from the record field to compare
 * @param value: The string value to compare against (converted to uint32_t)
 * @return true if values match, false otherwise
 */
bool Program_cmpUint32(const uint32_t recordValue, const char *value) {
    if (value == NULL || strcmp(value, "") == 0 || strcmp(value, "NULO") == 0) {
        return recordValue == EMPTY;
    }
    char *endptr;
    const long converted = strtol(value, &endptr, 10);
    if (*endptr != '\0') {
        // valor não é número válido
        return false;
    }
    return recordValue == (uint32_t)converted;
}

/**
 * @brief Compares a string field value with a search string.
 *
 * Treats empty string as NULL. Returns true if both are NULL or if the record string
 * matches the search string exactly.
 *
 * @param recordValue: The string value from the record field to compare
 * @param value: The string value to compare against
 * @return true if values match, false otherwise
 */
bool Program_cmpString(const char *recordValue, const char *value) {
    if (value == NULL || strcmp(value, "") == 0) {
        return recordValue == NULL;
    } else {
        return recordValue != NULL && strcmp(recordValue, value) == 0;
    }
}

/**
 * @brief Reads an integer from stdin and stores it as a string.
 *
 * Attempts to capture the next whitespace-delimited input from the user
 * and place it into the provided buffer. This allows numeric values or
 * the keyword "NULO" to be handled uniformly as strings for later parsing.
 * If the read fails, the buffer is set to an empty string to avoid
 * undefined content.
 *
 * @param value: Pointer to the character buffer where the input will be stored
 */
void Program_readIntAsString(char* value) {
    if (scanf("%s", value) != 1) {
        strcpy(value, "");
    }
}

/**
 * @brief Executes a search in the database based on user-defined criteria.
 *
 * Reads the number of criteria, then for each criterion reads the field name and value.
 * Uses ScanQuoteString to correctly parse string values (including quoted and NULO values).
 * Iterates through all records in the database, ignoring invalid or deleted ones, and checks
 * if they match all criteria. If a record matches, it is printed. If no records match, prints
 * "Registro inexistente.".
 *
 * @param dataBase: Pointer to the initialized database
 * @return true if search executed successfully, false otherwise
 */
bool Program_searchCriteria(const struct DataBase *dataBase) {
    if (dataBase == NULL) return false;
     bool printedAny = false;

    // Reads the number of criteria for the search
        uint32_t criteriaCount;
        if (scanf("%u", &criteriaCount) != 1) {
            throwError("Failed to read criteria count");
            return false;
        }

    // Reads the criteria for the search
        struct SearchCriteria criteria[criteriaCount];
        for (uint32_t j = 0; j < criteriaCount; j++) {

            // Reads the search field
            char field[INPUT_MAX_LENGTH];
            if (scanf("%s", field) != 1) {
                throwError("Failed to read search criteria field");
                return false;
            }

            // Defines which field the search criteria refers to and reads the criteria's value type (string with "" or integer as string)
            if (strcmp(field, "codEstacao") == 0) {
                criteria[j].field = StationID;
                Program_readIntAsString(criteria[j].value);
            } else if (strcmp(field, "nomeEstacao") == 0) {
                criteria[j].field = StationName;
                ScanQuoteString(criteria[j].value);
            } else if (strcmp(field, "codLinha") == 0) {
                criteria[j].field = LineID;
                Program_readIntAsString(criteria[j].value);
            } else if (strcmp(field, "nomeLinha") == 0) {
                criteria[j].field = LineName;
                ScanQuoteString(criteria[j].value);
            } else if (strcmp(field, "codProxEstacao") == 0) {
                criteria[j].field = DestinationStationID;
                Program_readIntAsString(criteria[j].value);
            } else if (strcmp(field, "distProxEstacao") == 0) {
                criteria[j].field = Distant;
                Program_readIntAsString(criteria[j].value);
            } else if (strcmp(field, "codEstIntegra") == 0) {
                criteria[j].field = InteractionStationID;
                Program_readIntAsString(criteria[j].value);
            } else if (strcmp(field, "codLinhaIntegra") == 0) {
                criteria[j].field = InteractionLineID;
                Program_readIntAsString(criteria[j].value);
            } else {
                throwError("Invalid search criteria field");
                return false;
            }
        }

        // Iterates through all records in database
        for (uint32_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
            struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, rrn);
            if (record == NULL) continue;  // removed or invalid

            // Checks each record field present in the search criteria
            // If any do not match, skip to the next record
            bool match = true;
            for (uint32_t j = 0; j < criteriaCount; j++) {
               const String value = criteria[j].value;
                switch (criteria[j].field) {
                    case StationID: {
                        if (!Program_cmpUint32(record->originStationID, value)) match = false;
                        break;
                    }
                    case StationName: {
                        if (!Program_cmpString(record->stationName, value)) match = false;
                        break;
                    }
                    case LineID: {
                        if (!Program_cmpUint32(record->originLineID, value)) match = false;
                        break;
                    }
                    case LineName: {
                        if (!Program_cmpString(record->lineName, value)) match = false;
                        break;
                    }
                    case DestinationStationID: {
                        if (!Program_cmpUint32(record->destinationStationID, value)) match = false;
                        break;
                    }
                    case Distant: {
                        if (!Program_cmpUint32(record->destinationDistant, value)) match = false;
                        break;
                    }
                    case InteractionStationID: {
                        if (!Program_cmpUint32(record->interactionStationID, value)) match = false;
                        break;
                    }
                    case InteractionLineID: {
                        if (!Program_cmpUint32(record->interactionLineID, value)) match = false;
                        break;
                    }
                    default:
                        match = false;
                        break;
                }

                if (!match) break;
            }

            // If a record is found, print it
            if (match) {
                Program_printRecord(record);
                printedAny = true;
            }

            SubwayRecord_free(record);
        }

    // If there are no records founded, print message
    if (!printedAny) printf("Registro inexistente.\n");
    return true;
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
    // Read the file paths
    Program_requestInput("Enter the input and output files paths:");
    char inputFilePath[INPUT_MAX_LENGTH], outputFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s", inputFilePath, outputFilePath) != 2) {
        throwError("Can not reading paths files");
        return false;
    }

    // Initialize database
    struct DataBase *dataBase = DataBaseRepository_init(outputFilePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    // Initialize input file
    struct InputFile *inputFile = InputRepository_openFile(inputFilePath);
    if (inputFile == NULL) {
        throwError("Failed to open file");
        return false;
    }

    // Extract records from the input file and save to the database while there are records
    struct SubwayRecord *record;
    while ((record = InputRepository_extractRecord(inputFile)) != NULL) {
        if (!DataBaseRepository_createRecord(dataBase, record)) {
            throwError("Failed to create record in data base");
            SubwayRecord_free(record);
            return false;
        }
        SubwayRecord_free(record);
    }

    // Finish procedure
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
    // Read the binary file path
    Program_requestInput("Enter the file path:");
    char filePath[INPUT_MAX_LENGTH];
    if (scanf("%s", filePath) != 1) {
        throwError("Failed read file path");
        return false;
    }

    // Initialize database
    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    // Print each record from the database
    bool printedRecord = false;
    for (int i = 0; i < dataBase->dataHeader->nextInsert; i++) {
        struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, i);
        if (record == NULL) continue;  // removed or invalid
        Program_printRecord(record);
        printedRecord = true;
        SubwayRecord_free(record);
    }

    // If there are no records, print message
    if (!printedRecord) printf("Registro inexistente.\n");

    // Finish procedure
    DataBaseRepository_close(dataBase);
    return true;
}


/**
 * @brief Performs multiple searches in the database.
 *
 * Reads the binary file path and the number of searches to perform. For each search,
 * calls Program_searchCriteria to process the criteria and display matching records.
 * If no records match any search, prints "Registro inexistente.".
 *
 * @return true if searches executed successfully, false otherwise
 */
bool Program_searchRecord() {
    // Reads the binary file path and the number of searches
    Program_requestInput("Enter the file path and number of searches:");
    uint32_t searchesCount;
    char filePath[INPUT_MAX_LENGTH];
    if (scanf("%s %u", filePath, &searchesCount) != 2) {
        throwError("Failed to read file path and searches count");
        return false;
    }

    // If there are no searches, do nothing
    if (searchesCount == 0) return true;

    // Initializes the database
    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        if (!Program_searchCriteria(dataBase)) {
            throwError("Failed to search record");
            DataBaseRepository_close(dataBase);
            return false;
        }
    }

    // Finalizes procedure
    DataBaseRepository_close(dataBase);
    return true;
}

/**
 * @brief Retrieves and displays a specific record by its RRN.
 * @return true if record was found and displayed, false if RRN invalid or error
 */
bool Program_getRecordByRRN() {
    uint32_t rrn;
    char filePath[INPUT_MAX_LENGTH];
    // Read the binary file path and the RRN of the desired record
    Program_requestInput("Enter the file path and RRN:");
    if (scanf("%s %u", filePath, &rrn) != 2) {
        throwError("Failed read file path and RRN");
        return false;
    }

    // Initialize database
    struct DataBase* dataBase = DataBaseRepository_init(filePath);
    if (dataBase == NULL) {
        throwError("Failed to initialize data base");
        return false;
    }

    // Read the record and print it if found, otherwise say record does not exist
    struct SubwayRecord *record = DataBaseRepository_readRecord(dataBase, rrn);
    if (record == NULL) {  // removed or invalid
        printf("Registro inexistente.\n");
    } else {
        Program_printRecord(record);
        SubwayRecord_free(record);
    }

    // Finish procedure
    DataBaseRepository_close(dataBase);
    return true;
}