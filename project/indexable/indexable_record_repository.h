//
// Created by arthu on 08/06/2026.
//

#ifndef INDEXABLE_RECORD_REPOSITORY_H
#define INDEXABLE_RECORD_REPOSITORY_H
#include "../services/file/file_repository.h"
#include <stdbool.h>
#include <stdint.h>

struct IndexableRecord {
    uint32_t rrn;
    uint32_t stationID;
};

bool IndexableRecordRepository_writeRecord(struct DataFile *dataFile, const struct IndexableRecord *record);
bool IndexableRecordRepository_readRecord(struct DataFile *dataFile, struct IndexableRecord *record);

#endif //INDEXABLE_RECORD_REPOSITORY_H
