#ifndef RECORD_REPOSITORY_H
#define RECORD_REPOSITORY_H

#include "../../domain/subway_record.h"
#include "../../core/file/file_repository.h"

bool RecordRepository_isRecordValid(const struct SubwayRecord *record);

bool RecordRepository_isRemoved(struct DataFile *dataFile, uint32_t rrn, bool *removed);

bool RecordRepository_removeRecord(struct DataFile *dataFile, uint32_t rrn, uint32_t *lastRemoved);

bool RecordRepository_readLastRemoved(struct DataFile *dataFile, uint32_t rrn, uint32_t *lastRemoved);

struct SubwayRecord *RecordRepository_readRecord(struct DataFile *dataFile, uint32_t rrn);

bool RecordRepository_writeRecord(struct DataFile *dataFile, struct SubwayRecord *record);


#endif //RECORD_REPOSITORY_H
