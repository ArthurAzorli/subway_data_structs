#include "program.h"
#include "services/file/file_repository.h"
#include "subway/subway_header_repository.h"
#include "subway/subway_record_repository.h"
#include "subway/input/input_repository.h"
#include "subway/search_criteria.h"
#include "lib/subway_record_list.h"
#include "lib/provided.h"

#include <stdio.h>
#include <stdlib.h>


#define INPUT_MAX_LENGTH 101
#define SUBWAY_HEADER_OFFSET 0
#define SUBWAY_RECORD_OFFSET 16
#define SUBWAY_RECORD_SIZE 80


// ===== Private Program Functions ===== \\

void Program_countStations(
    struct SubwayRecordList *recordList,
    struct SubwayRecord *record,
    uint32_t *stationsCount,
    uint32_t *pairStationsCount
) {
    *stationsCount = 0;
    *pairStationsCount = 0;

    if (recordList == NULL || record == NULL) return;
    if (SubwayRecordList_getSize(recordList) == 0) return;

    struct SubwayRecord *other = malloc(sizeof(struct SubwayRecord));
    if (other == NULL) return;

    for (size_t i = 0; i < SubwayRecordList_getSize(recordList); i++) {
        if (!SubwayRecordList_get(recordList, i, other)) break;
        if (other->stationNameLength == record->stationNameLength &&
            strcmp(other->stationName, record->stationName) == 0) {
            *stationsCount += 1;
        }
        if (record->originStationID == other->originStationID &&
            record->destinationStationID == other->destinationStationID) {
            *pairStationsCount += 1;
        }
    }
    free(other);
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
    return recordValue == (uint32_t) converted;
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
void Program_readIntAsString(char *value) {
    if (scanf("%s", value) != 1) {
        strcpy(value, "");
    }
}

bool Program_searchCriteria(const struct DataSubwayHeader *header, struct DataFile *file) {
    if (header == NULL || file == NULL) return false;
    bool printedAny = false;

    // Reads the number of criteria for the search
    uint32_t criteriaCount;
    if (scanf("%u", &criteriaCount) != 1)return false;


    // Reads the criteria for the search
    uint32_t position = 0;
    struct SearchCriteria criteria[criteriaCount];
    for (uint32_t j = 0; j < criteriaCount; j++) {
        // Reads the search field
        char field[INPUT_MAX_LENGTH];
        if (scanf("%s", field) != 1) return false;

        // Defines which field the search criteria refers to and reads the criteria's value type (string with "" or integer as string)
        if (strcmp(field, "codEstacao") == 0) {
            criteria[criteriaCount - 1].field = StationID;
            Program_readIntAsString(criteria[j].value);
        } else if (strcmp(field, "nomeEstacao") == 0) {
            criteria[position++].field = StationName;
            ScanQuoteString(criteria[position++].value);
        } else if (strcmp(field, "codLinha") == 0) {
            criteria[position++].field = LineID;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "nomeLinha") == 0) {
            criteria[position++].field = LineName;
            ScanQuoteString(criteria[position++].value);
        } else if (strcmp(field, "codProxEstacao") == 0) {
            criteria[position++].field = DestinationStationID;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "distProxEstacao") == 0) {
            criteria[position++].field = Distant;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "codEstIntegra") == 0) {
            criteria[position++].field = InteractionStationID;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "codLinhaIntegra") == 0) {
            criteria[position++].field = InteractionLineID;
            Program_readIntAsString(criteria[position++].value);
        } else {
            return false;
        }
    }

    // Iterates through all records in database
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(file);
        if (record == NULL) continue; // removed or invalid

        // Checks each record field present in the search criteria
        // If any do not match, skip to the next record
        bool match = false;
        bool uniqueFound = false;
        for (uint32_t j = 0; j < criteriaCount; j++) {
            const char *value = criteria[j].value;

            switch (criteria[j].field) {
                case StationID: {
                    match = Program_cmpUint32(record->originStationID, value);
                    uniqueFound = match;
                    break;
                }
                case StationName: {
                    match = Program_cmpString(record->stationName, value);
                    break;
                }
                case LineID: {
                    match = Program_cmpUint32(record->originLineID, value);
                    break;
                }
                case LineName: {
                    match = Program_cmpString(record->lineName, value);
                    break;
                }
                case DestinationStationID: {
                    match = Program_cmpUint32(record->destinationStationID, value);
                    break;
                }
                case Distant: {
                    match = Program_cmpUint32(record->destinationDistant, value);
                    break;
                }
                case InteractionStationID: {
                    match = Program_cmpUint32(record->interactionStationID, value);
                    break;
                }
                case InteractionLineID: {
                    match = Program_cmpUint32(record->interactionLineID, value);
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
        if (uniqueFound) return true;
    }

    // If there are no records founded, print message
    if (!printedAny) printf("Registro inexistente.\n");
    return true;
}


// ===== Public Program Functions ===== \\


bool Program_initSubwayFile() {
    //read files paths
    char inputFilePath[INPUT_MAX_LENGTH], outputFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s", inputFilePath, outputFilePath) != 2) return false;


    //init subway file header data
    struct DataSubwayHeader *header = SubwayHeaderRepository_init();
    if (header == NULL) return false;

    // create records list
    struct SubwayRecordList *recordList = SubwayRecordList_init();
    if (!recordList) {
        free(header);
        return false;
    }

    // open input file
    struct InputFile *inputFile = InputRepository_openFile(inputFilePath);
    if (inputFile == NULL) {
        SubwayRecordList_free(recordList);
        free(header);
        return false;
    }

    // Extract records from the input file and add in records list while there are records
    struct SubwayRecord *record;
    while ((record = InputRepository_extractRecord(inputFile)) != NULL) {
        uint32_t stationsCount = 0, pairStationsCount = 0;
        Program_countStations(recordList, record, &stationsCount, &pairStationsCount);

        //update header
        header->nextInsert++;
        if (stationsCount == 0) header->stationsCount++;
        if (record->destinationStationID != EMPTY && pairStationsCount == 0) header->pairStationsCount++;

        //add record on list
        SubwayRecordList_add(recordList, record);
        SubwayRecord_free(record);
    }

    //close input file
    InputRepository_closeFile(inputFile);

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(outputFilePath, WRITE_ONLY);
    if (dataFile == NULL) {
        SubwayRecordList_free(recordList);
        free(header);
        return false;
    }

    //write header on file
    if (!SubwayHeaderRepository_write(header, dataFile)) {
        FileRepository_close(dataFile);
        SubwayRecordList_free(recordList);
        free(header);
        return false;
    }

    struct SubwayRecord *record1 = malloc(sizeof(struct SubwayRecord));
    if (record1 == NULL) {
        FileRepository_close(dataFile);
        SubwayRecordList_free(recordList);
        free(header);
        return false;
    }

    // Write records on file
    for (size_t i = 0; i < SubwayRecordList_getSize(recordList); i++) {
        if (!SubwayRecordList_get(recordList, i, record1)) {
            FileRepository_close(dataFile);
            SubwayRecordList_free(recordList);
            free(header);
            free(record1);
            return false;
        }
        if (!SubwayRecordRepository_writeRecord(dataFile, record1)) {
            FileRepository_close(dataFile);
            SubwayRecordList_free(recordList);
            free(header);
            free(record1);
            return false;
        }
    }

    //close output file
    FileRepository_flush(dataFile);
    FileRepository_close(dataFile);

    //free memory
    SubwayRecordList_free(recordList);
    free(header);
    free(record1);

    BinarioNaTela(outputFilePath);
    return true;
}

bool Program_showRecords() {
    //read file path
    char filePath[INPUT_MAX_LENGTH];
    if (scanf("%s", filePath) != 1) return false;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(filePath, READ_ONLY);
    if (dataFile == NULL) return false;

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        FileRepository_close(dataFile);
        return false;
    }

    //read each record and prints
    for (size_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue;
        Program_printRecord(record);
        SubwayRecord_free(record);
    }

    //close file and free memory
    FileRepository_close(dataFile);
    free(header);
    return true;
}

bool Program_searchRecord() {
    uint32_t searchesCount;
    char filePath[INPUT_MAX_LENGTH];
    if (scanf("%s %u", filePath, &searchesCount) != 2) return false;

    // If there are no searches, do nothing
    if (searchesCount == 0) return true;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(filePath, READ_ONLY);
    if (dataFile == NULL) return false;

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        FileRepository_close(dataFile);
        return false;
    }

    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        if (!Program_searchCriteria(header, dataFile)) {
            FileRepository_close(dataFile);
            free(header);
            return false;
        }
        if (i < searchesCount - 1) {
            FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);
            printf("\n");
        }
    }

    // close file and finish memory
    FileRepository_close(dataFile);
    free(header);
    return true;
}

bool Program_getRecordByRRN() {
    uint32_t rrn;
    char filePath[INPUT_MAX_LENGTH];

    // Read the binary file path and the RRN of the desired record
    if (scanf("%s %u", filePath, &rrn) != 2) return false;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(filePath, READ_ONLY);
    if (dataFile == NULL) return false;

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        FileRepository_close(dataFile);
        return false;
    }

    if (header->nextInsert <= rrn) {
        printf("Registro inexistente.\n");
        FileRepository_close(dataFile);
        free(header);
        return true;
    }

    FileRepository_move(dataFile, SUBWAY_RECORD_SIZE * rrn);
    struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
    if (record == NULL) {
        printf("Registro inexistente.\n");
    } else {
        Program_printRecord(record);
        SubwayRecord_free(record);
    }

    //close file and free memory
    FileRepository_close(dataFile);
    free(header);
    return true;
}
