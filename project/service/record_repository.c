#include "record_repository.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/file/file_repository.h"

#define TRASH 0x24
#define RECORD_LENGTH 80
#define HEADER_LENGTH 16
#define RECORD_STATUS_LENGTH 5

long RecordRepository_getByteOffsetFromRRN(const uint32_t rrn) {
    return HEADER_LENGTH + RECORD_LENGTH * rrn;
}

bool RecordRepository_readString(struct DataFile *dataFile, uint32_t *length, char **result) {
    if (dataFile == NULL || length == NULL) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, length)) return false;
    *result = malloc(*length + 1);
    if (*result == NULL) {
        free(*result);
        printf("ERROR: Failed to allocate string\n");
        return false;
    }
    return FileRepository_readString(dataFile, *length, *result);
}

bool RecordRepository_writeString(struct DataFile *dataFile, size_t *remaining, uint32_t *length, const char *string) {
    if (dataFile == NULL || remaining == NULL || length == NULL || string == NULL) return false;
    if (*remaining < *length) {
        *length = *remaining;
    }

    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, *length)) return false;
    if (!FileRepository_writeString(dataFile, *length, string)) return false;
    *remaining -= *length;
    return true;
}

bool RecordRepository_readRecordData(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (record == NULL) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, &record->originStationID)) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, &record->originLineID)) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, &record->destinationStationID)) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, &record->destinationDistant)) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, &record->interactionLineID)) return false;
    if (!FileRepository_readInt(dataFile, BIG_ENDIAN, &record->interactionStationID)) return false;
    if (!RecordRepository_readString(dataFile, &record->stationNameLength, &record->stationName)) return false;
    if (!RecordRepository_readString(dataFile, &record->lineNameLength, &record->lineName)) return false;
    return true;
}

bool RecordRepository_writeRecordData(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (dataFile == NULL || record == NULL) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, record->originStationID)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, record->originLineID)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, record->destinationStationID)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, record->destinationDistant)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, record->interactionLineID)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, record->interactionStationID)) return false;
    size_t remaining = RECORD_LENGTH - 8 * UINT32_BYTES_COUNT;
    if (!RecordRepository_writeString(dataFile, &remaining, &record->stationNameLength, record->stationName)) {
        return false;
    }
    if (!RecordRepository_writeString(dataFile, &remaining, &record->lineNameLength, record->lineName)) {
        return false;
    }

    while (remaining > 0) {
        if (!FileRepository_writeByte(dataFile, TRASH)) return false;
        remaining--;
    }
    return true;
}

bool RecordRepository_isRecordValid(const struct SubwayRecord *record) {
    if (record == NULL) return false;
    if (record->originStationID == EMPTY) return false;
    if (record->stationNameLength == 0 || record->stationNameLength == EMPTY) return false;
    if (record->stationName == NULL) return false;
    return true;
}

bool RecordRepository_isRemoved(struct DataFile *dataFile, const uint32_t rrn, bool *removed) {
    if (dataFile == NULL) return false;
    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataFile, offset)) return false;
    return FileRepository_readBool(dataFile, removed);
}

bool RecordRepository_removeRecord(struct DataFile *dataFile, const uint32_t rrn, uint32_t* lastRemoved) {
    if (dataFile == NULL) return false;

    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed)) return false;
    if (removed) return true;

    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_moveUntil(dataFile, offset)) return false;
    if (!FileRepository_writeBool( dataFile, true)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, *lastRemoved)) return false;
    *lastRemoved = rrn;
    return true;
}

bool RecordRepository_readLastRemoved(struct DataFile *dataFile, const uint32_t rrn, uint32_t *lastRemoved) {
    if (dataFile == NULL) return false;

    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed) || !removed) return false;

    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_moveUntil(dataFile, offset + UINT8_BYTES_COUNT)) return false;
    return FileRepository_readInt(dataFile, BIG_ENDIAN, lastRemoved);
}

struct SubwayRecord *RecordRepository_readRecord(struct DataFile *dataFile, const uint32_t rrn) {
    if (dataFile == NULL) return NULL;
    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataFile, offset)) return NULL;


    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed) || removed) return NULL;
    if (!FileRepository_moveUntil(dataFile, offset + RECORD_STATUS_LENGTH)) return NULL;

    struct SubwayRecord *record = SubwayRecord_init();
    if (record == NULL) return NULL;
    if (!RecordRepository_readRecordData(dataFile, record)) {
        SubwayRecord_free(record);
        return NULL;
    }

    FileRepository_moveUntil(dataFile, offset + RECORD_LENGTH);
    return record;
}


bool RecordRepository_writeRecord(struct DataFile *dataFile, struct SubwayRecord *record) {
    if (dataFile == NULL) return false;
    if (!RecordRepository_isRecordValid(record)) {
        printf("ERROR: Record is not a valid subway record\n");
        return false;
    }
    if (record->rrn == EMPTY) {
        printf("ERROR: Record RRN is not set\n");
        return false;
    }
    const long offset = RecordRepository_getByteOffsetFromRRN(record->rrn);
    if (!FileRepository_goTo(dataFile, offset)) return false;

    if (!FileRepository_writeBool(dataFile, false)) return false;
    if (!FileRepository_writeInt(dataFile, BIG_ENDIAN, EMPTY)) return false;
    if (!RecordRepository_writeRecordData(dataFile, record)) return false;

    return true;
}
