#include "program.h"
#include "services/file/file_repository.h"
#include "subway/subway_header_repository.h"
#include "subway/subway_record_repository.h"
#include "subway/input/input_repository.h"
#include "subway/search_criteria.h"
#include "lib/subway_record_list.h"
#include "lib/indexable_record_avl.h"
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
    const struct SubwayRecord *record,
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


uint32_t Program_parseUint32(const char *value) {
    if (value == NULL || strcmp(value, "") == 0 || strcmp(value, "NULO") == 0) return EMPTY;
    char *endptr;
    const long converted = strtol(value, &endptr, 10);
    if (*endptr != '\0') return EMPTY;
    return (uint32_t) converted;
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

struct IndexableRecordAVL *Program_readIndexableAVL(const char *fileName) {
    if (fileName == NULL) return NULL;

    struct IndexableRecordAVL *avl = NULL;
    struct DataFile *file = FileRepository_openOrCreate(fileName, READ_ONLY);
    if (file == NULL) return NULL;

    struct IndexableRecord *indexable = NULL;
    while (true) {
        if (!IndexableRecordRepository_readRecord(file, &indexable)) break;
        avl = IndexableRecordAVL_push(avl, indexable);
        free(indexable);
    }

    return avl;
}

bool Program_readSearchCriteria(struct SearchCriteria *criteria, size_t *criteriaCount, bool *searchByID) {
    // Reads the number of criteria for the search
    if (scanf("%zu", criteriaCount) != 1) return false;
    *searchByID = false;

    // Reads the criteria for the search
    uint32_t position = 0;
    for (uint32_t j = 0; j < *criteriaCount; j++) {
        // Reads the search field
        char field[INPUT_MAX_LENGTH];
        if (scanf("%s", field) != 1) return false;

        // Defines which field the search criteria refers to and reads the criteria's value type (string with "" or integer as string)
        if (strcmp(field, "codEstacao") == 0) {
            *searchByID = true;
            criteria[*criteriaCount - 1].field = StationID;
            Program_readIntAsString(criteria[*criteriaCount - 1].value);
        } else if (strcmp(field, "nomeEstacao") == 0) {
            criteria[position].field = StationName;
            ScanQuoteString(criteria[position++].value);
        } else if (strcmp(field, "codLinha") == 0) {
            criteria[position].field = LineID;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "nomeLinha") == 0) {
            criteria[position].field = LineName;
            ScanQuoteString(criteria[position++].value);
        } else if (strcmp(field, "codProxEstacao") == 0) {
            criteria[position].field = DestinationStationID;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "distProxEstacao") == 0) {
            criteria[position].field = Distant;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "codEstIntegra") == 0) {
            criteria[position].field = InteractionStationID;
            Program_readIntAsString(criteria[position++].value);
        } else if (strcmp(field, "codLinhaIntegra") == 0) {
            criteria[position].field = InteractionLineID;
            Program_readIntAsString(criteria[position++].value);
        } else {
            return false;
        }
    }
    return true;
}

bool Program_recordMatchesCriteria(const struct SubwayRecord *record, struct SearchCriteria *criteria,
                                   size_t criteriaCount) {
    if (record == NULL || criteria == NULL) return false;

    for (size_t j = 0; j < criteriaCount; j++) {
        const char *value = criteria[j].value;
        bool match = false;

        switch (criteria[j].field) {
            case StationID: {
                const uint32_t stationID = Program_parseUint32(value);
                match = record->originStationID == stationID;
                break;
            }
            case StationName: {
                match = Program_cmpString(record->stationName, value);
                break;
            }
            case LineID: {
                const uint32_t lineID = Program_parseUint32(value);
                match = record->originLineID == lineID;
                break;
            }
            case LineName: {
                match = Program_cmpString(record->lineName, value);
                break;
            }
            case DestinationStationID: {
                const uint32_t destinationStationID = Program_parseUint32(value);
                match = destinationStationID == record->destinationStationID;
                break;
            }
            case Distant: {
                const uint32_t distant = Program_parseUint32(value);
                match = distant == record->destinationDistant;
                break;
            }
            case InteractionStationID: {
                const uint32_t interactionStationID = Program_parseUint32(value);
                match = interactionStationID == record->interactionStationID;
                break;
            }
            case InteractionLineID: {
                const uint32_t interactionLineID = Program_parseUint32(value);
                match = interactionLineID == record->interactionLineID;
                break;
            }
            default:
                match = false;
                break;
        }

        if (!match) return false;
    }

    return true;
}


bool Program_searchCriteriaByIndexable(const struct DataSubwayHeader *header, struct DataFile *file,
                                       const struct IndexableRecordAVL *avl,
                                       struct SubwayRecordList *subwayList) {
    if (file == NULL || avl == NULL || subwayList == NULL) return false;

    // Reads  criteria for the search
    bool searchByID;
    size_t criteriaCount;
    struct SearchCriteria *criteria = calloc(8, sizeof(struct SearchCriteria));
    if (criteria == NULL) return false;

    if (!Program_readSearchCriteria(criteria, &criteriaCount, &searchByID)) {
        free(criteria);
        return false;
    }

    if (searchByID) {
        const uint32_t stationID = Program_parseUint32(criteria[criteriaCount - 1].value);

        const struct IndexableRecord *indexable = IndexableRecordAVL_getByStationID(avl, stationID);
        if (indexable == NULL) {
            free(criteria);
            return true;
        }

        const uint32_t byteOffset = indexable->rrn * SUBWAY_RECORD_SIZE + SUBWAY_RECORD_OFFSET;
        FileRepository_goto(file, (long) byteOffset);
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(file);
        if (record == NULL) {
            free(criteria);
            return false;
        }

        //verify if fields matches without station ID
        if (Program_recordMatchesCriteria(record, criteria, criteriaCount - 1)) {
            record->rrn = indexable->rrn;
            SubwayRecordList_add(subwayList, record);
        }

        SubwayRecord_free(record);
    } else {
        // Iterates through all records in database
        for (uint32_t i = 0; i < header->nextInsert; i++) {
            struct SubwayRecord *record = SubwayRecordRepository_readRecord(file);
            if (record == NULL) continue; // removed or invalid

            // Checks each record field present in the search criteria
            // If any do not match, skip to the next record, else add it
            if (Program_recordMatchesCriteria(record, criteria, criteriaCount)) {
                record->rrn = i;
                SubwayRecordList_add(subwayList, record);
            }
            SubwayRecord_free(record);
        }

    }

    free(criteria);
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
    FileRepository_close(dataFile);

    //free memory
    SubwayRecordList_free(recordList);
    free(header);
    free(record1);

    //show binary
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
        bool printedAny = false;

        // Reads  criteria for the search
        bool searchByID;
        size_t criteriaCount;
        struct SearchCriteria *criteria = calloc(8, sizeof(struct SearchCriteria));
        if (criteria == NULL) {
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        if (!Program_readSearchCriteria(criteria, &criteriaCount, &searchByID)) {
            FileRepository_close(dataFile);
            free(header);
            free(criteria);
            return false;
        }

        // Iterates through all records in database
        for (uint32_t j = 0; j < header->nextInsert; j++) {
            struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
            if (record == NULL) continue; // removed or invalid

            // Checks each record field present in the search criteria
            // If any do not match, skip to the next record
            const bool match = Program_recordMatchesCriteria(record, criteria, criteriaCount);

            if (match) {
                Program_printRecord(record);
                printedAny = true;
            }

            SubwayRecord_free(record);
            //stop search when found stationID
            if (match && searchByID) break;
        }

        // If there are no records founded, print message
        if (!printedAny) printf("Registro inexistente.\n");


        if (i < searchesCount - 1) {
            FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);
            printf("\n");
        }
        free(criteria);
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

bool Program_initIndexableFile() {
    //read files paths
    char inputFilePath[INPUT_MAX_LENGTH], outputFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s", inputFilePath, outputFilePath) != 2) return false;

    struct IndexableRecordAVL *avl = NULL;

    //open input file
    struct DataFile *inputFile = FileRepository_openOrCreate(inputFilePath, READ_ONLY);
    if (inputFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(inputFile);
    if (header == NULL) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(inputFile);
        return false;
    }

    //read each record and prints
    for (size_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(inputFile);
        if (record == NULL) continue;
        struct IndexableRecord index = {i, record->originStationID};
        avl = IndexableRecordAVL_push(avl, &index);
        SubwayRecord_free(record);
    }

    //close input file
    FileRepository_close(inputFile);

    //catch avl error
    if (avl == NULL) {
        free(header);
        return false;
    }

    //open output file
    struct DataFile *outputFile = FileRepository_openOrCreate(outputFilePath, WRITE_ONLY);
    if (outputFile == NULL) {
        IndexableRecordAVL_free(avl);
        free(header);
        return false;
    }

    //write each AVL indexable record in file
    for (size_t i = 0; i < avl->size; i++) {
        const struct IndexableRecord *index = IndexableRecordAVL_getByIndex(avl, i);
        if (index == NULL) continue;
        if (!IndexableRecordRepository_writeRecord(outputFile, index)) {
            FileRepository_close(outputFile);
            IndexableRecordAVL_free(avl);
            free(header);
            return false;
        }
    }

    //close outputfile and free memory
    FileRepository_close(outputFile);
    IndexableRecordAVL_free(avl);
    free(header);

    //show binary
    BinarioNaTela(outputFilePath);
    return true;
}

bool Program_searchRecordByIndexable() {
    //read files paths and searches count
    uint32_t searchesCount;
    char indexFilePath[INPUT_MAX_LENGTH], subwayFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s %u", &subwayFilePath, &indexFilePath, &searchesCount) != 3) return false;

    // If there are no searches, do nothing
    if (searchesCount == 0) return true;

    struct IndexableRecordAVL *avl = Program_readIndexableAVL(indexFilePath);
    if (avl == NULL) return false;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(subwayFilePath, READ_ONLY);
    if (dataFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        return false;
    }

    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        struct SubwayRecordList *result = SubwayRecordList_init();
        if (result == NULL) {
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        if (!Program_searchCriteriaByIndexable(header, dataFile, avl, result)) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        const size_t resultCounts = SubwayRecordList_getSize(result);
        if (resultCounts > 0) {
            for (size_t j = 0; j < resultCounts; j++) {
                struct SubwayRecord *record = malloc(sizeof(struct SubwayRecord));
                if (!SubwayRecordList_get(result, j, record)) continue;
                Program_printRecord(record);
                SubwayRecord_free(record);
            }
        } else {
            printf("Registro inexistente.\n");
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

bool Program_removeRecords() {
    //read files paths and searches count
    uint32_t searchesCount;
    char indexFilePath[INPUT_MAX_LENGTH], subwayFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s %u", &subwayFilePath, &indexFilePath, &searchesCount) != 3) return false;

    // If there are no searches, do nothing
    if (searchesCount == 0) return true;

    struct IndexableRecordAVL *avl = Program_readIndexableAVL(indexFilePath);
    if (avl == NULL) return false;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(subwayFilePath, READ_WRITE);
    if (dataFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        return false;
    }

    struct SubwayRecordList *subwayList = SubwayRecordList_init();
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue; // removed or invalid

        SubwayRecordList_add(subwayList, record);
        SubwayRecord_free(record);
    }

    FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);


    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        struct SubwayRecordList *result = SubwayRecordList_init();
        if (result == NULL) {
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        if (!Program_searchCriteriaByIndexable(header, dataFile, avl, result)) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        const size_t resultCounts = SubwayRecordList_getSize(result);

        for (size_t j = 0; j < resultCounts; j++) {
            struct SubwayRecord *record = malloc(sizeof(struct SubwayRecord));
            if (!SubwayRecordList_get(result, j, record)) continue;

            const uint32_t byteOffset = record->rrn * SUBWAY_RECORD_SIZE + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, (long) byteOffset);

            //logic remove record on file and remove on list and avl
            SubwayRecordRepository_removeRecord(dataFile, header->lastRemoved);
            SubwayRecordList_removeByStationID(subwayList, record->originStationID);
            IndexableRecordAVL_remove(avl, record->originStationID);

            uint32_t stationsCount = 0, pairStationsCount = 0;
            Program_countStations(subwayList, record, &stationsCount, &pairStationsCount);

            //update header
            header->lastRemoved = record->rrn;
            if (stationsCount == 0) header->stationsCount--;
            if (record->destinationStationID != EMPTY && pairStationsCount == 0) header->pairStationsCount--;

            SubwayRecord_free(record);
        }


        if (i < searchesCount - 1) {
            FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);
        }
    }

    FileRepository_goto(dataFile, 0);
    SubwayHeaderRepository_write(header, dataFile);


    // close file and finish memory
    FileRepository_close(dataFile);
    free(header);

    //open output file
    struct DataFile *indexFile = FileRepository_openOrCreate(indexFilePath, WRITE_ONLY);
    if (indexFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //write each AVL indexable record in file
    for (size_t i = 0; i < avl->size; i++) {
        const struct IndexableRecord *index = IndexableRecordAVL_getByIndex(avl, i);
        if (index == NULL) continue;
        if (!IndexableRecordRepository_writeRecord(indexFile, index)) {
            FileRepository_close(indexFile);
            IndexableRecordAVL_free(avl);
            return false;
        }
    }

    // close file and finish memory
    FileRepository_close(indexFile);
    IndexableRecordAVL_free(avl);

    //print binary
    BinarioNaTela(subwayFilePath);
    BinarioNaTela(indexFilePath);
    return true;
}

bool Program_insertRecord() {
    //read files paths and searches count
    uint32_t searchesCount;
    char indexFilePath[INPUT_MAX_LENGTH], subwayFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s %u", &subwayFilePath, &indexFilePath, &searchesCount) != 3) return false;

    // If there are no searches, do nothing
    if (searchesCount == 0) return true;

    struct IndexableRecordAVL *avl = Program_readIndexableAVL(indexFilePath);
    if (avl == NULL) return false;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(subwayFilePath, READ_WRITE);
    if (dataFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        return false;
    }

    struct SubwayRecordList *subwayList = SubwayRecordList_init();
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue; // removed or invalid

        SubwayRecordList_add(subwayList, record);
        SubwayRecord_free(record);
    }

    for (uint32_t i = 0; i < searchesCount; i++) {
        struct SubwayRecord *record = SubwayRecord_init();
        if (record == NULL) {
            FileRepository_close(dataFile);
            SubwayRecordList_free(subwayList);
            IndexableRecordAVL_free(avl);
            free(header);
            return false;
        }

        char stationID[INPUT_MAX_LENGTH], lineID[INPUT_MAX_LENGTH], destinationID[INPUT_MAX_LENGTH];
        char distance[INPUT_MAX_LENGTH], interationStationID[INPUT_MAX_LENGTH], interactionLineID[INPUT_MAX_LENGTH];

        Program_readIntAsString(stationID);
        record->originStationID = Program_parseUint32(stationID);

        record->stationName = calloc(INPUT_MAX_LENGTH, sizeof(char));
        ScanQuoteString(record->stationName);
        record->stationNameLength = strlen(record->stationName);

        Program_readIntAsString(lineID);
        record->originLineID = Program_parseUint32(lineID);

        record->lineName = calloc(INPUT_MAX_LENGTH, sizeof(char));
        ScanQuoteString(record->lineName);
        record->lineNameLength = strlen(record->lineName);

        Program_readIntAsString(destinationID);
        record->destinationStationID = Program_parseUint32(destinationID);

        Program_readIntAsString(distance);
        record->destinationDistant = Program_parseUint32(distance);

        Program_readIntAsString(interationStationID);
        record->interactionStationID = Program_parseUint32(interationStationID);

        Program_readIntAsString(interactionLineID);
        record->interactionLineID = Program_parseUint32(interactionLineID);

        if (header->lastRemoved == EMPTY) {
            const long byteOffset = SUBWAY_RECORD_SIZE * header->nextInsert + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, byteOffset);
            record->rrn = header->nextInsert;
            header->nextInsert++;
        } else {
            const long byteOffset = SUBWAY_RECORD_SIZE * header->lastRemoved + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, byteOffset);
            record->rrn = header->lastRemoved;

            if (!SubwayRecordRepository_readNextRemoved(dataFile, &header->lastRemoved)) {
                FileRepository_close(dataFile);
                SubwayRecordList_free(subwayList);
                IndexableRecordAVL_free(avl);
                free(header);
                free(record);
                return false;
            }

            FileRepository_goto(dataFile, byteOffset);
            struct IndexableRecord index = {header->nextInsert, record->originStationID};
            IndexableRecordAVL_push(avl, &index);
        }

        if (!SubwayRecordRepository_writeRecord(dataFile, record)) {
            FileRepository_close(dataFile);
            SubwayRecordList_free(subwayList);
            IndexableRecordAVL_free(avl);
            free(header);
            free(record);
            return false;
        }

        uint32_t stationsCount = 0, pairStationsCount = 0;
        Program_countStations(subwayList, record, &stationsCount, &pairStationsCount);

        //update header
        if (stationsCount == 0) header->stationsCount++;
        if (record->destinationStationID != EMPTY && pairStationsCount == 0) header->pairStationsCount++;

        SubwayRecordList_add(subwayList, record);
        struct IndexableRecord index = {record->rrn, record->originStationID};
        IndexableRecordAVL_remove(avl, record->originStationID);
        IndexableRecordAVL_push(avl, &index);

        free(record);
    }

    FileRepository_goto(dataFile, 0);
    SubwayHeaderRepository_write(header, dataFile);

    FileRepository_close(dataFile);
    SubwayRecordList_free(subwayList);
    free(header);


    //open output file
    struct DataFile *indexFile = FileRepository_openOrCreate(indexFilePath, WRITE_ONLY);
    if (indexFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //write each AVL indexable record in file
    for (size_t i = 0; i < avl->size; i++) {
        const struct IndexableRecord *index = IndexableRecordAVL_getByIndex(avl, i);
        if (index == NULL) continue;
        if (!IndexableRecordRepository_writeRecord(indexFile, index)) {
            FileRepository_close(indexFile);
            IndexableRecordAVL_free(avl);
            return false;
        }
    }

    // close file and finish memory
    FileRepository_close(indexFile);
    IndexableRecordAVL_free(avl);

    //print binary
    BinarioNaTela(subwayFilePath);
    BinarioNaTela(indexFilePath);

    return true;
}

bool Program_updateRecord() {
    //read files paths and searches count
    uint32_t searchesCount;
    char indexFilePath[INPUT_MAX_LENGTH], subwayFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s %u", &subwayFilePath, &indexFilePath, &searchesCount) != 3) return false;

    // If there are no searches, do nothing
    if (searchesCount == 0) return true;

    struct IndexableRecordAVL *avl = Program_readIndexableAVL(indexFilePath);
    if (avl == NULL) return false;

    //open output file
    struct DataFile *dataFile = FileRepository_openOrCreate(subwayFilePath, READ_WRITE);
    if (dataFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //read header
    struct DataSubwayHeader *header = SubwayHeaderRepository_read(dataFile);
    if (header == NULL) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        return false;
    }

    struct SubwayRecordList *subwayList = SubwayRecordList_init();
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue; // removed or invalid

        SubwayRecordList_add(subwayList, record);
        SubwayRecord_free(record);
    }

    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);
        struct SubwayRecordList *result = SubwayRecordList_init();
        if (result == NULL) {
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        if (!Program_searchCriteriaByIndexable(header, dataFile, avl, result)) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        // Reads  criteria for the search
        bool searchByID;
        size_t criteriaCount;
        struct SearchCriteria *criteria = calloc(8, sizeof(struct SearchCriteria));
        if (criteria == NULL) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }


        if (!Program_readSearchCriteria(criteria, &criteriaCount, &searchByID)) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            free(criteria);
            return false;
        }

        if (criteriaCount == 0) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            free(criteria);
            return true;
        }

        const size_t resultCounts = SubwayRecordList_getSize(result);

        for (size_t j = 0; j < resultCounts; j++) {
            struct SubwayRecord *record = malloc(sizeof(struct SubwayRecord));
            if (!SubwayRecordList_get(result, j, record)) continue;

            const uint32_t byteOffset = record->rrn * SUBWAY_RECORD_SIZE + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, (long) byteOffset);

            if (searchByID) IndexableRecordAVL_remove(avl, record->originStationID);
            SubwayRecordList_removeByStationID(subwayList, record->originStationID);

            uint32_t oldStationsCount = 0, oldPairStationsCount = 0;
            Program_countStations(subwayList, record, &oldStationsCount, &oldPairStationsCount);

            if (oldStationsCount == 0) header->stationsCount--;
            if (record->destinationStationID != EMPTY && oldPairStationsCount == 0) header->pairStationsCount--;

            for (size_t k = 0; k < criteriaCount; k++) {
                const char *value = criteria[k].value;

                switch (criteria[k].field) {
                    case StationID: {
                        const uint32_t stationID = Program_parseUint32(value);
                        record->originStationID = stationID;
                        break;
                    }
                    case StationName: {
                        if (record->stationName != NULL) free(record->stationName);
                        record->stationName = strdup(value);
                        record->stationNameLength = strlen(value);
                        break;
                    }
                    case LineID: {
                        const uint32_t lineID = Program_parseUint32(value);
                        record->originLineID = lineID;
                        break;
                    }
                    case LineName: {
                        if (record->lineName != NULL) free(record->lineName);
                        free(record->lineName);
                        record->lineName = strdup(value);
                        record->lineNameLength = strlen(value);
                        break;
                    }
                    case DestinationStationID: {
                        const uint32_t destinationStationID = Program_parseUint32(value);
                        record->destinationStationID = destinationStationID;
                        break;
                    }
                    case Distant: {
                        const uint32_t distant = Program_parseUint32(value);
                        record->destinationDistant = distant;
                        break;
                    }
                    case InteractionStationID: {
                        const uint32_t interactionStationID = Program_parseUint32(value);
                        record->interactionStationID = interactionStationID;
                        break;
                    }
                    case InteractionLineID: {
                        const uint32_t interactionLineID = Program_parseUint32(value);
                        record->interactionLineID = interactionLineID;
                        break;
                    }
                    default:
                        break;
                }
            }


            SubwayRecordRepository_writeRecord(dataFile, record);
            uint32_t newStationsCount = 0, newPairStationsCount = 0;
            Program_countStations(subwayList, record, &newStationsCount, &newPairStationsCount);

            if (newStationsCount == 0) header->stationsCount++;
            if (record->destinationStationID != EMPTY && newPairStationsCount == 0) header->pairStationsCount++;
            SubwayRecordList_add(subwayList, record);

            if (searchByID) {
                struct IndexableRecord index = {record->rrn, record->originStationID};
                IndexableRecordAVL_push(avl, &index);
            }

            SubwayRecord_free(record);
        }

        SubwayRecordList_free(result);
        free(criteria);
    }

    FileRepository_goto(dataFile, 0);
    SubwayHeaderRepository_write(header, dataFile);

    FileRepository_close(dataFile);
    SubwayRecordList_free(subwayList);
    free(header);

    //open output file
    struct DataFile *indexFile = FileRepository_openOrCreate(indexFilePath, WRITE_ONLY);
    if (indexFile == NULL) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    //write each AVL indexable record in file
    for (size_t i = 0; i < avl->size; i++) {
        const struct IndexableRecord *index = IndexableRecordAVL_getByIndex(avl, i);
        if (index == NULL) continue;
        if (!IndexableRecordRepository_writeRecord(indexFile, index)) {
            FileRepository_close(indexFile);
            IndexableRecordAVL_free(avl);
            return false;
        }
    }

    // close file and finish memory
    FileRepository_close(indexFile);
    IndexableRecordAVL_free(avl);

    //print binary
    BinarioNaTela(subwayFilePath);
    BinarioNaTela(indexFilePath);

    return true;
}
