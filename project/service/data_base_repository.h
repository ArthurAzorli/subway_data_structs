#ifndef DATA_BASE_REPOSITORY_H
#define DATA_BASE_REPOSITORY_H

#include "../core/utils/types.h"
#include "../domain/subway_record.h"

struct DataBase {
    struct DataHeader *dataHeader;
    struct DataFile *dataFile;
};

struct DataBase *DataBaseRepository_init(String path);

bool DataBaseRepository_createRecord(const struct DataBase* dataBase, struct SubwayRecord *record);

struct SubwayRecord *DataBaseRepository_readRecord(const struct DataBase* dataBase, size_t rrn);

bool DataBaseRepository_updateRecord(const struct DataBase* dataBase, struct SubwayRecord *record);

bool DataBaseRepository_deleteRecord(const struct DataBase* dataBase, size_t rrn);

void DataBaseRepository_close(struct DataBase* dataBase);


#endif //DATA_BASE_REPOSITORY_H
