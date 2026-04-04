#include "record_repository.h"

#include <stdio.h>
#include <stdlib.h>

#include "../../core/file/file_repository.h"
#include "../../core/utils/errors.h"

#define TRASH 0x24
#define RECORD_LENGTH 80
#define RECORD_SECTION_OFFSET 16
#define RECORD_STATUS_LENGTH 5

/**
 * @brief Calculates the absolute byte offset for a record given its RRN.
 * @param rrn: The Relative Record Number
 * @return Absolute byte offset from start of record section
 */
long RecordRepository_getByteOffsetFromRRN(const uint32_t rrn) {
    return RECORD_SECTION_OFFSET + RECORD_LENGTH * rrn;
}

/**
 * @brief Reads a length-prefixed string from the file.
 *
 * Reads a uint32_t length value, allocates memory for the string,
 * reads the string bytes, and null-terminates the result.
 *
 * @param dataFile: Open data file to read from
 * @param length: Pointer to receive the string length
 * @param result: Pointer to receive the allocated string pointer
 * @return true if string was successfully read, false on error
 *
 * @note Caller must free the allocated string
 */
bool RecordRepository_readString(struct DataFile *dataFile, uint32_t *length, char **result) {
    if (dataFile == NULL || length == NULL) return false;
    // le o tamanho da string
    if (!FileRepository_readInt(dataFile, length)) return false;
    //aloca a memoria para a string
    *result = malloc(*length + 1);
    if (*result == NULL) {
        throwError("Failed to allocate string");
        return false;
    }
    //se for nao for vazia a le
    if (*length > 0) {
        if (!FileRepository_readString(dataFile, *length, *result)) {
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
 * Writes a uint32_t length value followed by the string bytes, respecting
 * the available remaining space in the record. If string exceeds space,
 * length is capped to available space.
 *
 * @param dataFile: Open data file to write to
 * @param remaining: Pointer to bytes remaining in current record (updated after write)
 * @param length: Pointer to string length (maybe adjusted to fit remaining space)
 * @param string: The string to write
 * @return true if write was successful, false otherwise
 */
bool RecordRepository_writeString(struct DataFile *dataFile, size_t *remaining, uint32_t *length, const char *string) {
    if (dataFile == NULL || remaining == NULL || length == NULL) return false;
    if (*remaining < *length) {
        *length = *remaining;
    }

    if (!FileRepository_writeInt(dataFile, *length)) return false;
    if (*length > 0 && string != NULL) {
        if (!FileRepository_writeString(dataFile, *length, string)) return false;
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
bool RecordRepository_readRecordData(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (record == NULL) return false;
    if (!FileRepository_readInt(dataFile, &record->originStationID)) return false;
    if (!FileRepository_readInt(dataFile, &record->originLineID)) return false;
    if (!FileRepository_readInt(dataFile, &record->destinationStationID)) return false;
    if (!FileRepository_readInt(dataFile, &record->destinationDistant)) return false;
    if (!FileRepository_readInt(dataFile, &record->interactionLineID)) return false;
    if (!FileRepository_readInt(dataFile, &record->interactionStationID)) return false;
    if (!RecordRepository_readString(dataFile, &record->stationNameLength, &record->stationName)) return false;
    if (!RecordRepository_readString(dataFile, &record->lineNameLength, &record->lineName)) return false;
    return true;
}

/**
 * @brief Writes all fields of a record to the file (excluding status).
 *
 * Writes 6 uint32_t IDs, then the variable-length station and line names,
 * finally padding with TRASH bytes to reach RECORD_LENGTH boundary.
 *
 * @param dataFile: Open data file to write to
 * @param record: Record structure with data to write
 * @return true if all fields were successfully written, false on error
 */
bool RecordRepository_writeRecordData(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (dataFile == NULL || record == NULL) return false;
    if (!FileRepository_writeInt(dataFile, record->originStationID)) return false;
    if (!FileRepository_writeInt(dataFile, record->originLineID)) return false;
    if (!FileRepository_writeInt(dataFile, record->destinationStationID)) return false;
    if (!FileRepository_writeInt(dataFile, record->destinationDistant)) return false;
    if (!FileRepository_writeInt(dataFile, record->interactionLineID)) return false;
    if (!FileRepository_writeInt(dataFile, record->interactionStationID)) return false;
    size_t remaining = RECORD_LENGTH - (8 * UINT32_BYTES_COUNT) - RECORD_STATUS_LENGTH;
    if (!RecordRepository_writeString(dataFile, &remaining, &record->stationNameLength, record->stationName)) {
        return false;
    }
    if (!RecordRepository_writeString(dataFile, &remaining, &record->lineNameLength, record->lineName)) {
        return false;
    }

    // o espaço restante do registro é preenchido por lixo '$'
    while (remaining > 0) {
        if (!FileRepository_writeByte(dataFile, TRASH)) return false;
        remaining--;
    }
    return true;
}

/**
 * @brief Validates that a SubwayRecord contains required data.
 * @param record: The record to validate
 * @return true if record meets minimum validity requirements, false otherwise
 */
bool RecordRepository_isRecordValid(const struct SubwayRecord *record) {
    if (record == NULL) return false;
    if (record->originStationID == EMPTY) return false;
    if (record->stationNameLength == 0 || record->stationNameLength == EMPTY) return false;
    if (record->stationName == NULL) return false;
    return true;
}

/**
 * @brief Checks if a record at a given RRN is marked as removed.
 * @param dataFile: Open data file to read from
 * @param rrn: Relative Record Number to check
 * @param removed: Pointer to boolean to receive the removal status
 * @return true if status was successfully read, false on error
 */
bool RecordRepository_isRemoved(struct DataFile *dataFile, const uint32_t rrn, bool *removed) {
    if (dataFile == NULL) return false;
    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataFile, offset)) return false;
    if (!FileRepository_readBool(dataFile, removed)) return false;
    return true;
}

/**
 * @brief Marks a record as removed and updates the removal chain.
 * @param dataFile: Open data file to write to
 * @param rrn: Relative Record Number to mark as removed
 * @param lastRemoved: Pointer to update with the new lastRemoved value
 * @return true if record was successfully marked, false otherwise
 */
bool RecordRepository_removeRecord(struct DataFile *dataFile, const uint32_t rrn, uint32_t *lastRemoved) {
    if (dataFile == NULL) return false;

    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed)) return false;
    if (removed) return true;

    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_moveUntil(dataFile, offset)) return false;
    if (!FileRepository_writeBool(dataFile, true)) return false;
    if (!FileRepository_writeInt(dataFile, *lastRemoved)) return false;
    *lastRemoved = rrn;
    return true;
}

/**
 * @brief Reads the next RRN from the removal chain.
 * @param dataFile: Open data file to read from
 * @param rrn: RRN of the removed record to read from
 * @param lastRemoved: Pointer to receive the next RRN in the chain
 * @return true if pointer was successfully read, false otherwise
 */
bool RecordRepository_readLastRemoved(struct DataFile *dataFile, const uint32_t rrn, uint32_t *lastRemoved) {
    if (dataFile == NULL) return false;

    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed) || !removed) return false;

    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_moveUntil(dataFile, offset + UINT8_BYTES_COUNT)) return false;
    return FileRepository_readInt(dataFile, lastRemoved);
}

/**
 * @brief Reads a complete record from the database file.
 *
 * Locates a record by its RRN, verifies it hasn't been removed, reads all
 * fields, sets the RRN field, and returns an allocated SubwayRecord structure.
 *
 * @param dataFile: Open data file to read from
 * @param rrn: Relative Record Number of the record to read
 * @return Pointer to allocated SubwayRecord on success, NULL if record is removed or read fails
 */
struct SubwayRecord *RecordRepository_readRecord(struct DataFile *dataFile, const uint32_t rrn) {
    if (dataFile == NULL) return NULL;
    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataFile, offset)) return NULL;

    //verifica se nao esta removido
    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed) || removed) return NULL;
    if (!FileRepository_moveUntil(dataFile, offset + RECORD_STATUS_LENGTH)) return NULL;

    struct SubwayRecord *record = SubwayRecord_init();
    if (record == NULL) return NULL;
    if (!RecordRepository_readRecordData(dataFile, record)) {
        SubwayRecord_free(record);
        return NULL;
    }

    //move para o proximo registro para otimizar leitura sequencial
    FileRepository_moveUntil(dataFile, offset + RECORD_LENGTH);
    return record;
}


/**
 * @brief Writes a complete record to the database file.
 *
 * Writes a record to its designated RRN location, including status byte,
 * all fields, and padding to maintain fixed record size.
 *
 * @param dataFile: Open data file to write to
 * @param record: The record to write (must have valid RRN and data)
 * @return true if record was successfully written, false otherwise
 */
bool RecordRepository_writeRecord(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (dataFile == NULL) return false;
    if (!RecordRepository_isRecordValid(record)) {
        throwError("Record is not a valid subway record");
        return false;
    }
    if (record->rrn == EMPTY) {
        throwError("Record RRN is not set");
        return false;
    }
    const long offset = RecordRepository_getByteOffsetFromRRN(record->rrn);
    if (!FileRepository_goTo(dataFile, offset)) return false;

    //inicializa os status da flag de removido e de proximo registro removido
    if (!FileRepository_writeBool(dataFile, false)) return false;
    if (!FileRepository_writeInt(dataFile, EMPTY)) return false;

    //escreve o resgistro
    if (!RecordRepository_writeRecordData(dataFile, record)) {
        throwError("Fail to write record data");
        return false;
    }

    return true;
}
