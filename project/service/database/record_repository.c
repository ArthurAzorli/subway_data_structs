#include "record_repository.h"

#include <stdio.h>
#include <stdlib.h>

#include "../../core/file/file_repository.h"

#define TRASH 0x24
#define RECORD_LENGTH 80
#define RECORD_SECTION_OFFSET 16
#define RECORD_STATUS_LENGTH 5

long RecordRepository_getByteOffsetFromRRN(const uint32_t rrn) {
    return RECORD_SECTION_OFFSET + RECORD_LENGTH * rrn;
}

bool RecordRepository_readString(struct DataFile *dataFile, uint32_t *length, char **result) {
    if (dataFile == NULL || length == NULL) return false;
    if (!FileRepository_readInt(dataFile, length)) return false;
    *result = malloc(*length + 1);
    if (*result == NULL) {
        printf("ERROR: Failed to allocate string\n");
        return false;
    }
    if (*length > 0) {
        if (!FileRepository_readString(dataFile, *length, *result)) {
            free(*result);
            return false;
        }
    }
    (*result)[*length] = '\0';
    return true;
}

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
    if (!FileRepository_readBool(dataFile, removed)) return false;
    return true;
}

bool RecordRepository_removeRecord(struct DataFile *dataFile, const uint32_t rrn, uint32_t* lastRemoved) {
    if (dataFile == NULL) return false;

    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed)) return false;
    if (removed) return true;

    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_moveUntil(dataFile, offset)) return false;
    if (!FileRepository_writeBool( dataFile, true)) return false;
    if (!FileRepository_writeInt(dataFile, *lastRemoved)) return false;
    *lastRemoved = rrn;
    return true;
}

bool RecordRepository_readLastRemoved(struct DataFile *dataFile, const uint32_t rrn, uint32_t *lastRemoved) {
    if (dataFile == NULL) return false;

    bool removed;
    if (!RecordRepository_isRemoved(dataFile, rrn, &removed) || !removed) return false;

    const long offset = RecordRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_moveUntil(dataFile, offset + UINT8_BYTES_COUNT)) return false;
    return FileRepository_readInt(dataFile, lastRemoved);
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
    if (!FileRepository_writeInt(dataFile, EMPTY)) return false;
    if (!RecordRepository_writeRecordData(dataFile, record)) {
        printf("ERROR: Fail to write record data\n");
        return false;
    }

    return true;
}
