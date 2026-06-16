#include "search_service.h"
#include "types.h"
#include  "../lib/provided.h"
#include "../subway/subway_record_repository.h"
#include "../indexable/indexable_record_repository.h"

#include <stdio.h>
#include <string.h>

#define FIELD_MAX_LENGTH 101

/**
 * @brief Reads subway search fields and their values from standard input.
 *
 * This function reads the number of search criteria, then iterates through each
 * field name and assigns the corresponding field type and value. If the field
 * is "codEstacao", it is placed at the last position to prioritize ID-based searches.
 *
 * @param fields Array of SubwayFieldValue structures to store the criteria
 * @param fieldsCount Pointer to store the number of fields read
 * @param hasStationID Pointer to flag whether StationID was included in the criteria
 * @return true if all fields were successfully read, false otherwise
 */
bool SearchService_readSubwayFields(struct SubwayFieldValue *fields, size_t *fieldsCount, bool *hasStationID) {
    // Reads the number of criteria for the search
    if (scanf("%zu", fieldsCount) != 1) return false;
    *hasStationID = false;

    // Reads each the criteria for the search
    uint32_t position = 0;
    for (uint32_t j = 0; j < *fieldsCount; j++) {
        // Reads the search field
        char field[FIELD_MAX_LENGTH];
        if (scanf("%s", field) != 1) return false;

        // Defines which field the search criteria refers to and reads the criteria's value type (string with "" or integer as string)
        if (strcmp(field, "codEstacao") == 0) {
            *hasStationID = true;
            //put station ID in last position
            fields[*fieldsCount - 1].field = StationID;
            readIntAsString(fields[*fieldsCount - 1].value);
        } else if (strcmp(field, "nomeEstacao") == 0) {
            fields[position].field = StationName;
            ScanQuoteString(fields[position++].value);
        } else if (strcmp(field, "codLinha") == 0) {
            fields[position].field = LineID;
            readIntAsString(fields[position++].value);
        } else if (strcmp(field, "nomeLinha") == 0) {
            fields[position].field = LineName;
            ScanQuoteString(fields[position++].value);
        } else if (strcmp(field, "codProxEstacao") == 0) {
            fields[position].field = DestinationStationID;
            readIntAsString(fields[position++].value);
        } else if (strcmp(field, "distProxEstacao") == 0) {
            fields[position].field = Distant;
            readIntAsString(fields[position++].value);
        } else if (strcmp(field, "codEstIntegra") == 0) {
            fields[position].field = InteractionStationID;
            readIntAsString(fields[position++].value);
        } else if (strcmp(field, "codLinhaIntegra") == 0) {
            fields[position].field = InteractionLineID;
            readIntAsString(fields[position++].value);
        } else {
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks if a subway record matches a set of search criteria.
 *
 * Iterates through all provided search fields and compares them against the
 * corresponding values in the record. Returns false if any field does not match.
 *
 * @param record Pointer to the SubwayRecord to check
 * @param fields Array of SubwayFieldValue criteria
 * @param fieldsCount Number of criteria in the array
 * @return true if the record matches all criteria, false otherwise
 */
bool SearchService_recordMatchesFields(const struct SubwayRecord *record, struct SubwayFieldValue *fields,
                                       size_t fieldsCount) {
    if (record == NULL || fields == NULL) return false;

    // compare each criteria fields
    for (size_t j = 0; j < fieldsCount; j++) {
        const char *value = fields[j].value;
        bool match = false;

        switch (fields[j].field) {
            case StationID: {
                const uint32_t stationID = parseUint32(value);
                match = record->originStationID == stationID;
                break;
            }
            case StationName: {
                match = cmpString(record->stationName, value);
                break;
            }
            case LineID: {
                const uint32_t lineID = parseUint32(value);
                match = record->originLineID == lineID;
                break;
            }
            case LineName: {
                match = cmpString(record->lineName, value);
                break;
            }
            case DestinationStationID: {
                const uint32_t destinationStationID = parseUint32(value);
                match = destinationStationID == record->destinationStationID;
                break;
            }
            case Distant: {
                const uint32_t distant = parseUint32(value);
                match = distant == record->destinationDistant;
                break;
            }
            case InteractionStationID: {
                const uint32_t interactionStationID = parseUint32(value);
                match = interactionStationID == record->interactionStationID;
                break;
            }
            case InteractionLineID: {
                const uint32_t interactionLineID = parseUint32(value);
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

/**
 * @brief Performs a sequential search over all subway records.
 *
 * Reads search criteria from input and iterates through all records in the database.
 * Adds matching records to the provided list. If the criteria include StationID,
 * the search stops after the first match.
 *
 * @param header Pointer to the DataSubwayHeader containing metadata
 * @param file Pointer to the DataFile representing the subway database
 * @param subwayList Pointer to the list where matching records will be stored
 * @return true if the search executed successfully, false otherwise
 */
bool SearchService_sequencialSearch(const struct DataSubwayHeader *header, struct DataFile *file,
                                    struct SubwayRecordList *subwayList) {

    if (header == NULL || file == NULL ||subwayList == NULL) return false;

    bool searchByID;
    size_t fieldsCount;

    //init fields list
    struct SubwayFieldValue *fields = calloc(FIELDS_MAX_COUNT, sizeof(struct SubwayFieldValue));
    if (fields == NULL) return false;

    // Reads search fields criteria
    if (!SearchService_readSubwayFields(fields, &fieldsCount, &searchByID)) {
        free(fields);
        return false;
    }

    // Iterates through all records in database
    for (uint32_t i = 0; i < header->nextInsert; i++) {
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(file);
        if (record == NULL) continue; // removed or invalid

        // Checks each record field present in the search criteria
        // If any do not match, skip to the next record, else add it
        if (SearchService_recordMatchesFields(record, fields, fieldsCount)) {
            record->rrn = i;
            SubwayRecordList_add(subwayList, record);

            //If matches and criteria contains station ID stop search
            if (searchByID) {
                SubwayRecord_free(record);
                free(fields);
                return true;
            }
        }
        SubwayRecord_free(record);
    }

    free(fields);
    return true;
}

/**
 * @brief Performs an indexable search using an AVL tree of station IDs.
 *
 * Reads search criteria from input. If StationID is included, uses the AVL index
 * to locate the record directly. Otherwise, falls back to sequential search.
 * Matching records are added to the provided list.
 *
 * @param header Pointer to the DataSubwayHeader containing metadata
 * @param file Pointer to the DataFile representing the subway database
 * @param avl Pointer to the IndexableRecordAVL structure containing indexed records
 * @param subwayList Pointer to the list where matching records will be stored
 * @return true if the search executed successfully, false otherwise
 */
bool SearchService_indexableSearch(const struct DataSubwayHeader *header, struct DataFile *file,
                                       const struct IndexableRecordAVL *avl,
                                       struct SubwayRecordList *subwayList) {
    if (file == NULL || avl == NULL || subwayList == NULL) return false;

    bool searchByID;
    size_t fieldsCount;

    //init fields list
    struct SubwayFieldValue *fields = calloc(FIELDS_MAX_COUNT, sizeof(struct SubwayFieldValue));
    if (fields == NULL) return false;

    // Reads search fields criteria
    if (!SearchService_readSubwayFields(fields, &fieldsCount, &searchByID)) {
        free(fields);
        return false;
    }

    //If criteria contains Station ID use indexes to do search, else do sequencial search
    if (searchByID) {
        //get subway index
        const uint32_t stationID = parseUint32(fields[fieldsCount - 1].value);
        const struct IndexableRecord *indexable = IndexableRecordAVL_getByStationID(avl, stationID);
        if (indexable == NULL) {
            //not found
            free(fields);
            return true;
        }

        // go to record byte offset
        const uint32_t byteOffset = indexable->rrn * SUBWAY_RECORD_SIZE + SUBWAY_RECORD_OFFSET;
        FileRepository_goto(file, (long) byteOffset);

        //read record
        struct SubwayRecord *record = SubwayRecordRepository_readRecord(file);
        if (record == NULL) {
            //inconsistent on subway and index files
            free(fields);
            return false;
        }

        //Checks others record fields matches
        //If all fields matches add it
        if (SearchService_recordMatchesFields(record, fields, fieldsCount - 1)) {
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
            if (SearchService_recordMatchesFields(record, fields, fieldsCount)) {
                record->rrn = i;
                SubwayRecordList_add(subwayList, record);
            }
            SubwayRecord_free(record);
        }
    }

    free(fields);
    return true;
}
