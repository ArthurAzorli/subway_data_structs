#include "program.h"
#include "services/file_repository.h"
#include "services/search_service.h"
#include "services/types.h"
#include "subway/subway_header_repository.h"
#include "subway/subway_record_repository.h"
#include "subway/input/input_repository.h"
#include "subway/subway_record_list.h"
#include "indexable/indexable_record_avl.h"
#include "lib/provided.h"


#include <stdio.h>
#include <stdlib.h>


#define INPUT_MAX_LENGTH 101
#define SUBWAY_HEADER_OFFSET 0


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

    struct SubwayRecord *other = SubwayRecord_init();
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
    SubwayRecord_free(other);
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

    struct SubwayRecord *record1 = SubwayRecord_init();
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
            SubwayRecord_free(record1);
            free(header);
            return false;
        }
        if (!SubwayRecordRepository_writeRecord(dataFile, record1)) {
            FileRepository_close(dataFile);
            SubwayRecordList_free(recordList);
            SubwayRecord_free(record1);
            free(header);
            return false;
        }
    }

    //close output file
    FileRepository_close(dataFile);

    //free memory
    SubwayRecordList_free(recordList);
    SubwayRecord_free(record1);
    free(header);


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
        SubwayRecord_print(record);
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
        struct SubwayRecordList *result = SubwayRecordList_init();
        if (result == NULL) {
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //do sequencial search
        if (!SearchService_sequencialSearch(header, dataFile, result)) {
            SubwayRecordList_free(result);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //print all records returned
        const size_t size = SubwayRecordList_getSize(result);
        for (size_t j = 0; j < size; j++) {
            struct SubwayRecord *record = SubwayRecord_init();
            if (!SubwayRecordList_get(result, j, record)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(result);
                FileRepository_close(dataFile);
                free(header);
                return false;
            }
            SubwayRecord_print(record);
            SubwayRecord_free(record);
        }

        // If there are no records founded, print message
        if (size == 0) printf("Registro inexistente.\n");

        // go to initial byte offset to a new search
        if (i < searchesCount - 1) {
            FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);
            printf("\n");
        }

        SubwayRecordList_free(result);
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
        SubwayRecord_print(record);
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

    //write index on output file
    if (!IndexableRecordAVL_writeOnFile(outputFilePath, avl)) {
        IndexableRecordAVL_free(avl);
        free(header);
        return false;
    }

    //close outputfile and free memory
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

    struct IndexableRecordAVL *avl = IndexableRecordAVL_readFromFile(indexFilePath);
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

        // do indexable search
        if (!SearchService_indexableSearch(header, dataFile, avl, result)) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //print all records returned
        const size_t size = SubwayRecordList_getSize(result);
        for (size_t j = 0; j < size; j++) {
            struct SubwayRecord *record = SubwayRecord_init();
            if (!SubwayRecordList_get(result, j, record)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(result);
                IndexableRecordAVL_free(avl);
                FileRepository_close(dataFile);
                free(header);
                return false;
            }
            SubwayRecord_print(record);
            SubwayRecord_free(record);
        }

        // If there are no records founded, print message
        if (size == 0) printf("Registro inexistente.\n");

        // go to initial byte offset to a new search
        if (i < searchesCount - 1) {
            FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);
            printf("\n");
        }

        SubwayRecordList_free(result);
    }

    // close file and finish memory
    IndexableRecordAVL_free(avl);
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

    struct IndexableRecordAVL *avl = IndexableRecordAVL_readFromFile(indexFilePath);
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

    //read all records to update header
    struct SubwayRecordList *subwayList = SubwayRecordList_init();
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue; // removed or invalid
        SubwayRecordList_add(subwayList, record);
        SubwayRecord_free(record);
    }

    //go to initial byteOffset
    FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);


    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        struct SubwayRecordList *result = SubwayRecordList_init();
        if (result == NULL) {
            SubwayRecordList_free(subwayList);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //do indexable search
        if (!SearchService_indexableSearch(header, dataFile, avl, result)) {
            SubwayRecordList_free(subwayList);
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        const size_t resultCounts = SubwayRecordList_getSize(result);
        for (size_t j = 0; j < resultCounts; j++) {
            struct SubwayRecord *record = SubwayRecord_init();
            if (!SubwayRecordList_get(result, j, record)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(subwayList);
                SubwayRecordList_free(result);
                IndexableRecordAVL_free(avl);
                FileRepository_close(dataFile);
                free(header);
                return false;
            }

            //go to record byteOffset
            const uint32_t byteOffset = record->rrn * SUBWAY_RECORD_SIZE + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, (long) byteOffset);

            //logic remove record on file
            if (!SubwayRecordRepository_removeRecord(dataFile, header->lastRemoved)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(subwayList);
                SubwayRecordList_free(result);
                IndexableRecordAVL_free(avl);
                FileRepository_close(dataFile);
                free(header);
                return false;
            }
            //remove record from list and avl
            SubwayRecordList_removeByStationID(subwayList, record->originStationID);
            avl = IndexableRecordAVL_remove(avl, record->originStationID);

            //calculate stations and pairs count
            uint32_t stationsCount = 0, pairStationsCount = 0;
            Program_countStations(subwayList, record, &stationsCount, &pairStationsCount);

            //update header fields
            header->lastRemoved = record->rrn;
            if (stationsCount == 0) header->stationsCount--;
            if (record->destinationStationID != EMPTY && pairStationsCount == 0) header->pairStationsCount--;

            SubwayRecord_free(record);
        }

        // go to initial byte offset to a new search
        if (i < searchesCount - 1) FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);

        SubwayRecordList_free(result);
    }

    SubwayRecordList_free(subwayList);

    //go to header offset and write it
    FileRepository_goto(dataFile, SUBWAY_HEADER_OFFSET);
    if (!SubwayHeaderRepository_write(header, dataFile)) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        free(header);
        return false;
    }

    // close file and finish memory
    FileRepository_close(dataFile);
    free(header);

    //write index file
    if (!IndexableRecordAVL_writeOnFile(indexFilePath, avl)) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    IndexableRecordAVL_free(avl);

    //print binary
    BinarioNaTela(subwayFilePath);
    BinarioNaTela(indexFilePath);
    return true;
}

bool Program_insertRecord() {
    //read files paths and inserts count
    uint32_t insertsCount;
    char indexFilePath[INPUT_MAX_LENGTH], subwayFilePath[INPUT_MAX_LENGTH];
    if (scanf("%s %s %u", &subwayFilePath, &indexFilePath, &insertsCount) != 3) return false;

    // If there are no searches, do nothing
    if (insertsCount == 0) return true;

    struct IndexableRecordAVL *avl = IndexableRecordAVL_readFromFile(indexFilePath);
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

    //read all records to update header
    struct SubwayRecordList *subwayList = SubwayRecordList_init();
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue; // removed or invalid
        SubwayRecordList_add(subwayList, record);
        SubwayRecord_free(record);
    }

    for (uint32_t i = 0; i < insertsCount; i++) {
        struct SubwayRecord *record = SubwayRecord_init();
        if (record == NULL) {
            SubwayRecordList_free(subwayList);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //read each record field
        char stationID[INPUT_MAX_LENGTH], lineID[INPUT_MAX_LENGTH], destinationID[INPUT_MAX_LENGTH];
        char distance[INPUT_MAX_LENGTH], interationStationID[INPUT_MAX_LENGTH], interactionLineID[INPUT_MAX_LENGTH];

        //read station ID
        readIntAsString(stationID);
        record->originStationID = parseUint32(stationID);

        //read station name
        record->stationName = calloc(INPUT_MAX_LENGTH, sizeof(char));
        ScanQuoteString(record->stationName);
        record->stationNameLength = strlen(record->stationName);

        //read line ID
        readIntAsString(lineID);
        record->originLineID = parseUint32(lineID);

        //read line name
        record->lineName = calloc(INPUT_MAX_LENGTH, sizeof(char));
        ScanQuoteString(record->lineName);
        record->lineNameLength = strlen(record->lineName);

        //read destination station ID
        readIntAsString(destinationID);
        record->destinationStationID = parseUint32(destinationID);

        //read destination distance
        readIntAsString(distance);
        record->destinationDistant = parseUint32(distance);

        //read interaction station ID
        readIntAsString(interationStationID);
        record->interactionStationID = parseUint32(interationStationID);

        //read interaction line ID
        readIntAsString(interactionLineID);
        record->interactionLineID = parseUint32(interactionLineID);

        //If has a removed station reuse its rrn, else add on end file
        if (header->lastRemoved == EMPTY) {
            const long byteOffset = SUBWAY_RECORD_SIZE * header->nextInsert + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, byteOffset);
            record->rrn = header->nextInsert;
            header->nextInsert++;
        } else {
            const long byteOffset = SUBWAY_RECORD_SIZE * header->lastRemoved + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, byteOffset);
            record->rrn = header->lastRemoved;

            //read next rrn removed to pile top
            if (!SubwayRecordRepository_readNextRemoved(dataFile, &header->lastRemoved)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(subwayList);
                IndexableRecordAVL_free(avl);
                FileRepository_close(dataFile);
                free(header);
                return false;
            }

            FileRepository_goto(dataFile, byteOffset);
        }

        //write new record fields
        if (!SubwayRecordRepository_writeRecord(dataFile, record)) {
            SubwayRecord_free(record);
            SubwayRecordList_free(subwayList);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //calculate stations and pairs count
        uint32_t stationsCount = 0, pairStationsCount = 0;
        Program_countStations(subwayList, record, &stationsCount, &pairStationsCount);

        //update header fields
        if (stationsCount == 0) header->stationsCount++;
        if (record->destinationStationID != EMPTY && pairStationsCount == 0) header->pairStationsCount++;

        //add record on list and overwrite record index
        struct IndexableRecord index = {record->rrn, record->originStationID};
        IndexableRecordAVL_remove(avl, record->originStationID);
        IndexableRecordAVL_push(avl, &index);
        SubwayRecordList_add(subwayList, record);

        SubwayRecord_free(record);
    }

    //go to header offset and write it
    FileRepository_goto(dataFile, SUBWAY_HEADER_OFFSET);
    if (!SubwayHeaderRepository_write(header, dataFile)) {
        SubwayRecordList_free(subwayList);
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        free(header);
        return false;
    }

    FileRepository_close(dataFile);
    SubwayRecordList_free(subwayList);
    free(header);

    //write index file
    if (!IndexableRecordAVL_writeOnFile(indexFilePath, avl)) {
        IndexableRecordAVL_free(avl);
        return false;
    }

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

    struct IndexableRecordAVL *avl = IndexableRecordAVL_readFromFile(indexFilePath);
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

    //read all records to update header
    struct SubwayRecordList *subwayList = SubwayRecordList_init();
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(dataFile);
        if (record == NULL) continue; // removed or invalid
        SubwayRecordList_add(subwayList, record);
        SubwayRecord_free(record);
    }

    //go to initial byteOffset
    FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);

    // Performs n searches
    for (uint32_t i = 0; i < searchesCount; i++) {
        struct SubwayRecordList *result = SubwayRecordList_init();
        if (result == NULL) {
            SubwayRecordList_free(subwayList);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        //do indexable search
        if (!SearchService_indexableSearch(header, dataFile, avl, result)) {
            SubwayRecordList_free(subwayList);
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        bool hasStationID;
        size_t fieldsCount;

        //init fields list
        struct SubwayFieldValue *fields = calloc(FIELDS_MAX_COUNT, sizeof(struct SubwayFieldValue));
        if (fields == NULL) {
            SubwayRecordList_free(subwayList);
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            return false;
        }

        // Reads search fields criteria
        if (!SearchService_readSubwayFields(fields, &fieldsCount, &hasStationID)) {
            SubwayRecordList_free(result);
            IndexableRecordAVL_free(avl);
            FileRepository_close(dataFile);
            free(header);
            free(fields);
            return false;
        }

        // nobody will update
        if (fieldsCount == 0) {
            SubwayRecordList_free(result);
            free(fields);
            continue;
        }

        const size_t resultCounts = SubwayRecordList_getSize(result);
        for (size_t j = 0; j < resultCounts; j++) {
            struct SubwayRecord *record = SubwayRecord_init();
            if (!SubwayRecordList_get(result, j, record)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(subwayList);
                SubwayRecordList_free(result);
                IndexableRecordAVL_free(avl);
                FileRepository_close(dataFile);
                free(header);
                free(fields);
                return false;
            }

            //go to record byteOffset
            const uint32_t byteOffset = record->rrn * SUBWAY_RECORD_SIZE + SUBWAY_RECORD_OFFSET;
            FileRepository_goto(dataFile, (long) byteOffset);

            //remove record from list and index from avl (if has station ID on updated fields)
            if (hasStationID) IndexableRecordAVL_remove(avl, record->originStationID);
            SubwayRecordList_removeByStationID(subwayList, record->originStationID);

            //calculate stations and pairs count when removed old record fields
            uint32_t oldStationsCount = 0, oldPairStationsCount = 0;
            Program_countStations(subwayList, record, &oldStationsCount, &oldPairStationsCount);

            //update header fields
            if (oldStationsCount == 0) header->stationsCount--;
            if (record->destinationStationID != EMPTY && oldPairStationsCount == 0) header->pairStationsCount--;

            //update record fields values
            for (size_t k = 0; k < fieldsCount; k++) {
                const char *value = fields[k].value;

                switch (fields[k].field) {
                    case StationID: {
                        const uint32_t stationID = parseUint32(value);
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
                        const uint32_t lineID = parseUint32(value);
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
                        const uint32_t destinationStationID = parseUint32(value);
                        record->destinationStationID = destinationStationID;
                        break;
                    }
                    case Distant: {
                        const uint32_t distant = parseUint32(value);
                        record->destinationDistant = distant;
                        break;
                    }
                    case InteractionStationID: {
                        const uint32_t interactionStationID = parseUint32(value);
                        record->interactionStationID = interactionStationID;
                        break;
                    }
                    case InteractionLineID: {
                        const uint32_t interactionLineID = parseUint32(value);
                        record->interactionLineID = interactionLineID;
                        break;
                    }
                    default:
                        break;
                }
            }

            //write new values record on file
            if (!SubwayRecordRepository_writeRecord(dataFile, record)) {
                SubwayRecord_free(record);
                SubwayRecordList_free(subwayList);
                SubwayRecordList_free(result);
                IndexableRecordAVL_free(avl);
                FileRepository_close(dataFile);
                free(header);
                free(fields);
                return false;
            }

            //calculate stations and pairs count after changes
            uint32_t newStationsCount = 0, newPairStationsCount = 0;
            Program_countStations(subwayList, record, &newStationsCount, &newPairStationsCount);

            //update header fields
            if (newStationsCount == 0) header->stationsCount++;
            if (record->destinationStationID != EMPTY && newPairStationsCount == 0) header->pairStationsCount++;
            SubwayRecordList_add(subwayList, record);

            //add new index after changes
            if (hasStationID) {
                struct IndexableRecord index = {record->rrn, record->originStationID};
                IndexableRecordAVL_push(avl, &index);
            }

            SubwayRecord_free(record);
        }

        // go to initial byte offset to a new search
        if (i < searchesCount - 1) FileRepository_goto(dataFile, SUBWAY_RECORD_OFFSET);

        SubwayRecordList_free(result);
        free(fields);
    }

    SubwayRecordList_free(subwayList);

    //go to header offset and write it
    FileRepository_goto(dataFile, SUBWAY_HEADER_OFFSET);
    if (!SubwayHeaderRepository_write(header, dataFile)) {
        IndexableRecordAVL_free(avl);
        FileRepository_close(dataFile);
        free(header);
        return false;
    }

    // close file and finish memory
    FileRepository_close(dataFile);
    free(header);

    //write index file
    if (!IndexableRecordAVL_writeOnFile(indexFilePath, avl)) {
        IndexableRecordAVL_free(avl);
        return false;
    }

    IndexableRecordAVL_free(avl);

    //print binary
    BinarioNaTela(subwayFilePath);
    BinarioNaTela(indexFilePath);
    return true;
}
