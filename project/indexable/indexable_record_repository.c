#include "indexable_record_repository.h"

#include "../services/file/file_repository.h"
#include <stdlib.h>

bool IndexableRecordRepository_writeRecord(struct DataFile *dataFile, const struct IndexableRecord *record) {
    if (dataFile == NULL || record == NULL) return false;

    if (!FileRepository_write(dataFile, INTEGER, &record->stationID, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->rrn, 1)) return false;
    return true;
}

bool IndexableRecordRepository_readRecord(struct DataFile *dataFile, struct IndexableRecord **record) {
    if (dataFile == NULL) return false;

    *record = malloc(sizeof(struct IndexableRecord));
    if (*record == NULL) return false;
    if (!FileRepository_read(dataFile, INTEGER, &(*record)->stationID, 1)) {
        free(*record);
        return false;
    }
    if (!FileRepository_read(dataFile, INTEGER, &(*record)->rrn, 1)) {
        free(*record);
        return false;
    }
    return true;
}
