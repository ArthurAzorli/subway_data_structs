/**
 * @file header_repository.h
 * @brief Management of database file header information.
 *
 * This module handles reading and writing of the database header which contains
 * metadata about the database state.
 */

#ifndef TRABALHO01_HEADER_REPOSITORY_H
#define TRABALHO01_HEADER_REPOSITORY_H

#include "../../core/file/file_repository.h"
#include "../../core/utils/types.h"

/**
 * @struct DataHeader
 * @brief Metadata header for the database file.
 */
struct DataHeader {
    uint32_t nextInsert; /**< RRN (Relative Record Number) for the next new record insertion */
    uint32_t lastRemoved; /**< RRN of the most recently removed record (for reuse chain) */
    uint32_t stationsCount; /**< Total count of unique station names in database */
    uint32_t pairStationsCount; /**< Total count of unique station pairs (routes) in database */
};

struct DataHeader *HeaderRepository_init(struct DataFile *dataFile);

bool HeaderRepository_save(const struct DataHeader *header, struct DataFile *dataFile);


#endif //TRABALHO01_HEADER_REPOSITORY_H
