/**
 * @file data_base_repository.h
 * @brief High-level database operations and CRUD interface.
 *
 * This module provides the main API for database operations including initialization,
 * record creation, reading, updating, deletion, and closure. It manages both
 * file and header repositories.
 */

#ifndef DATA_BASE_REPOSITORY_H
#define DATA_BASE_REPOSITORY_H

#include "../../core/utils/types.h"
#include "../../domain/subway_record.h"

/**
 * @struct DataBase
 * @brief Represents the entire database system.
 *
 * Contains references to both the header file and data file repositories
 * which together form a complete database instance.
 */
struct DataBase {
    struct DataHeader *dataHeader; /**< Pointer to the database header with metadata */
    struct DataFile *dataFile; /**< Pointer to the binary data file */
};

struct DataBase *DataBaseRepository_init(String path);

bool DataBaseRepository_createRecord(const struct DataBase *dataBase, struct SubwayRecord *record);

struct SubwayRecord *DataBaseRepository_readRecord(const struct DataBase *dataBase, size_t rrn);

bool DataBaseRepository_updateRecord(const struct DataBase *dataBase, struct SubwayRecord *record);

bool DataBaseRepository_deleteRecord(const struct DataBase *dataBase, size_t rrn);

void DataBaseRepository_close(struct DataBase *dataBase);


#endif //DATA_BASE_REPOSITORY_H
