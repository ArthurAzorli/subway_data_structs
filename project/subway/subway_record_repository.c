#include "subway_record_repository.h"

#include <stdlib.h>
#include <string.h>

#include "../services/file/file_repository.h"
#include "../services/utils/types.h"


#define TRASH 0x24
#define RECORD_LENGTH 80
#define TRUE_MARK '1'
#define FALSE_MARK '0'

// ======= Public Subway Record Functions ======= \\

/**
 * @brief Allocates and initializes a new SubwayRecord structure.
 * @return Pointer to allocated SubwayRecord on success, NULL on allocation failure
 * @note Sets all numeric fields to EMPTY and all pointer fields to NULL
 */
struct SubwayRecord *SubwayRecord_init() {
    struct SubwayRecord *record = malloc(sizeof(struct SubwayRecord));
    if (record == NULL) return NULL;

    // Initialize the fields
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
 * @brief Frees all resources associated with a SubwayRecord.
 * @param record: The record to free (can be NULL)
 */
void SubwayRecord_free(struct SubwayRecord *record) {
    if (record == NULL) return;
    if (record->stationName != NULL) free(record->stationName);
    if (record->lineName != NULL) free(record->lineName);
    free(record);
}

// ======= Private Subway Record Repository Functions ======= \\

bool SubwayRecordRepository_isNextRemoved(struct DataFile *dataFile) {
    uint8_t removedMark = TRUE_MARK;
    FileRepository_read(dataFile, BOOLEAN, &removedMark, 1);
    return removedMark != FALSE_MARK;
}

/**
 * @brief Reads a length-prefixed string from the file.
 *
 * Reads an uint32_t length value, allocates memory for the string,
 * reads the string bytes, and null-terminates the result.
 *
 * @param dataFile: Open data file to read from
 * @param length: Pointer to receive the string length
 * @param result: Pointer to receive the allocated string pointer
 * @return true if string was successfully read, false on error
 *
 * @note Caller must free the allocated string
 */
bool SubwayRecordRepository_readString(struct DataFile *dataFile, uint32_t *length, char **result) {
    if (dataFile == NULL || length == NULL) return false;
    // Read the string length
    if (!FileRepository_read(dataFile, INTEGER, length, 1)) return false;
    // Allocate memory for the string
    *result = malloc(*length + 1);
    if (*result == NULL) return false;

    // If it is not empty, read it
    if (*length > 0) {
        if (!FileRepository_read(dataFile, CHAR, *result, *length)) {
            free(*result);
            return false;
        }
    }
    (*result)[*length] = '\0';
    return true;
}

/**
 * @brief Writes a length-prefixed string to the file with space limitation.
 *
 * Writes an uint32_t length value followed by the string bytes, respecting
 * the available remaining space in the record. If string exceeds space,
 * length is capped to available space.
 *
 * @param dataFile: Open data file to write to
 * @param remaining: Pointer to bytes remaining in current record (updated after write)
 * @param length: Pointer to string length (maybe adjusted to fit remaining space)
 * @param string: The string to write
 * @return true if write was successful, false otherwise
 */
bool SubwayRecordRepository_writeString(struct DataFile *dataFile, size_t *remaining, uint32_t *length,
                                        const char *string) {
    if (dataFile == NULL || remaining == NULL || length == NULL) return false;
    if (*remaining < *length) *length = *remaining;

    if (!FileRepository_write(dataFile, INTEGER, length, 1)) return false;
    if (*length > 0 && string != NULL) {
        if (!FileRepository_write(dataFile, CHAR, string, *length)) return false;
    }
    *remaining -= *length;
    return true;
}

/**
 * @brief Reads all fields of a record from the file (excluding status).
 *
 * Reads 6 uint32_t IDs, then the variable-length station and line names.
 * Does not read the record status byte.
 *
 * @param dataFile: Open data file to read from
 * @param record: Allocated record structure to populate
 * @return true if all fields were successfully read, false on error
 */
bool SubwayRecordRepository_readRecordData(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (record == NULL) return false;
    if (!FileRepository_read(dataFile, INTEGER, &record->originStationID, 1)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &record->originLineID, 1)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &record->destinationStationID, 1)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &record->destinationDistant, 1)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &record->interactionLineID, 1)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &record->interactionStationID, 1)) return false;
    if (!SubwayRecordRepository_readString(dataFile, &record->stationNameLength, &record->stationName)) return false;
    if (!SubwayRecordRepository_readString(dataFile, &record->lineNameLength, &record->lineName)) {
        free(record->stationName);
        record->stationName = NULL;
        return false;
    }
    const size_t remaining = RECORD_LENGTH - 37 - record->stationNameLength - record->lineNameLength;
    FileRepository_read(dataFile, INTEGER, &record->originStationID, remaining);
    return true;
}


// ======= Public Subway Record Repository Functions ======= \\

struct SubwayRecord *SubwayRecordRepository_readRecord(struct DataFile *dataFile) {
    if (dataFile == NULL) return NULL;

    if (SubwayRecordRepository_isNextRemoved(dataFile)) {
        FileRepository_move(dataFile, RECORD_LENGTH - 1);
        return NULL;
    }

    FileRepository_move(dataFile, 4);
    struct SubwayRecord *record = SubwayRecord_init();
    if (record == NULL) return NULL;

    if (!SubwayRecordRepository_readRecordData(dataFile, record)) {
        SubwayRecord_free(record);
        return NULL;
    }

    return record;
}

bool SubwayRecordRepository_writeRecord(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (dataFile == NULL || record == NULL) return false;

    if (record->originStationID == EMPTY) return false;
    if (record->stationNameLength == 0 || record->stationNameLength == EMPTY) return false;
    if (record->stationName == NULL) return false;

    const uint8_t removedMark = FALSE_MARK;
    const uint32_t nextRemoved = EMPTY;
    if (!FileRepository_write(dataFile, BOOLEAN, &removedMark, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &nextRemoved, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->originStationID, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->originLineID, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->destinationStationID, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->destinationDistant, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->interactionLineID, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->interactionStationID, 1)) return false;
    size_t remaining = RECORD_LENGTH - 37;
    if (!SubwayRecordRepository_writeString(dataFile, &remaining, &record->stationNameLength, record->stationName)) {
        return false;
    }
    if (!SubwayRecordRepository_writeString(dataFile, &remaining, &record->lineNameLength, record->lineName)) {
        return false;
    }

    // the remaining space of the record is filled with trash '$'
    const char trash = TRASH;
    while (remaining > 0) {
        if (!FileRepository_write(dataFile, CHAR, &trash, 1)) return false;
        remaining--;
    }

    return true;
}

bool SubwayRecordRepository_removeRecord(struct DataFile *dataFile, const uint32_t lastRemoved) {
    if (dataFile == NULL) return false;
    const uint8_t removedMark = TRUE_MARK;
    if (!FileRepository_write(dataFile, CHAR, &removedMark, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &lastRemoved, 1)) return false;
    FileRepository_move(dataFile, RECORD_LENGTH - 5);
    return true;
}

bool SubwayRecordRepository_readNextRemoved(struct DataFile *dataFile, uint32_t *nextRemoved) {
    if (dataFile == NULL) return false;
    if (!SubwayRecordRepository_isNextRemoved(dataFile)) return false;
    return FileRepository_read(dataFile, INTEGER, nextRemoved, 1);
}
