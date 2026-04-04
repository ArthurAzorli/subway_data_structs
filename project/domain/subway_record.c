#include "subway_record.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/utils/types.h"
#include "../core/utils/errors.h"

/**
 * @brief Allocates and initializes a new SubwayRecord structure.
 * @return Pointer to allocated SubwayRecord on success, NULL on allocation failure
 * @note Sets all numeric fields to EMPTY and all pointer fields to NULL
 */
struct SubwayRecord *SubwayRecord_init() {
    struct SubwayRecord *record = malloc(sizeof(struct SubwayRecord));
    if (record == NULL) {
        throwError("ERROR: Failed to allocate data record\n");
        return NULL;
    }
    //start the fiedls
    record->rrn = EMPTY;
    record->originStationID = EMPTY;
    record->originLineID = EMPTY;
    record->destinationStationID = EMPTY;
    record->destinationDistant = EMPTY;
    record->interactionLineID = EMPTY;
    record->interactionStationID = EMPTY;
    record->stationNameLength = 0;
    record->stationName = NULL;
    record->lineNameLength = 0;
    record->lineName = NULL;
    return record;
}

/**
 * @brief Compares two subway records for equality.
 * @param thisRecord: First record to compare
 * @param otherRecord: Second record to compare
 * @return true if both records are equal, false otherwise or if either is NULL
 */
bool SubwayRecord_isEquals(const struct SubwayRecord *thisRecord, const struct SubwayRecord *otherRecord) {
    if (thisRecord == NULL || otherRecord == NULL) return false;
    if (thisRecord->originStationID != otherRecord->originStationID) return false;
    if (thisRecord->originLineID != otherRecord->originLineID) return false;
    if (thisRecord->destinationStationID != otherRecord->destinationStationID) return false;
    if (thisRecord->destinationDistant != otherRecord->destinationDistant) return false;
    if (thisRecord->interactionStationID != otherRecord->interactionStationID) return false;
    if (thisRecord->interactionLineID != otherRecord->interactionLineID) return false;
    //se as strings nao forem null compara com strcmp
    if (thisRecord->stationName != NULL && otherRecord->stationName != NULL) {
        if (strcmp(thisRecord->stationName, otherRecord->stationName) != 0) return false;
    } else if (thisRecord->stationName != otherRecord->stationName) {
        return false;
    }
    if (thisRecord->lineName != NULL && otherRecord->lineName != NULL) {
        if (strcmp(thisRecord->lineName, otherRecord->lineName) != 0) return false;
    } else if (thisRecord->lineName != otherRecord->lineName) {
        return false;
    }
    return true;
}

/**
 * @brief Frees all resources associated with a SubwayRecord.
 * @param record: The record to free (can be NULL)
 */
void SubwayRecord_free(struct SubwayRecord *record) {
    if (record == NULL) return;
    if (record->stationName != NULL) free(record->stationName);
    if (record->lineName != NULL) free(record->lineName);
    free(record);
}
