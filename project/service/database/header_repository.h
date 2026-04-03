//
// Created by User on 28/03/2026.
//

#ifndef TRABALHO01_HEADER_REPOSITORY_H
#define TRABALHO01_HEADER_REPOSITORY_H

#include "../../core/file/file_repository.h"
#include "../../core/utils/types.h"

struct DataHeader {
    uint32_t nextInsert;
    uint32_t lastRemoved;
    uint32_t stationsCount;
    uint32_t pairStationsCount;
};

struct DataHeader *HeaderRepository_init(struct DataFile *dataFile);

bool HeaderRepository_save(const struct DataHeader *header, struct DataFile *dataFile);


#endif //TRABALHO01_HEADER_REPOSITORY_H
