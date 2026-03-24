#ifndef DATA_BASE_REPOSITORY_H
#define DATA_BASE_REPOSITORY_H

#include "../core/utils/types.h"
#include "../domain/subway_line_record.h"

struct DataBase;

struct DataBase *DataBaseRepository_init(String path);

bool DataBaseRepository_createRecord(struct DataBase* dataBase, struct SubwayLineRecord *record);

struct SubwayLineRecord *DataBaseRepository_readRecord(const struct DataBase* dataBase, size_t rrn);

bool DataBaseRepository_updateRecord(struct DataBase* dataBase, struct SubwayLineRecord *record);

bool DataBaseRepository_deleteRecord(struct DataBase* dataBase, size_t rrn);

bool DataBaseRepository_existRecord(const struct DataBase* dataBase, String stationName);

void DataBaseRepository_close(struct DataBase* dataBase);


#endif //DATA_BASE_REPOSITORY_H
