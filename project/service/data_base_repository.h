#ifndef DATA_BASE_REPOSITORY_H
#define DATA_BASE_REPOSITORY_H

#include "../core/utils/types.h"
#include "../domain/subway_line_record.h"

#define EMPTY 0xFFFFFFFF


struct DataFile* DataBaseRepository_init(String path);

bool DataBaseRepository_createRecord(struct DataFile dataBase, struct SubwayLineRecord *record);

struct SubwayLineRecord *DataBaseRepository_readRecord(struct DataFile dataBase, const size_t byteOffset);

bool DataBaseRepository_updateRecord(struct DataFile dataBase, const struct SubwayLineRecord *record);

bool DataBaseRepository_deleteRecord(struct DataFile dataBase, const size_t byteOffset);

void DataBaseRepository_close(struct DataFile dataBase);


#endif //DATA_BASE_REPOSITORY_H
