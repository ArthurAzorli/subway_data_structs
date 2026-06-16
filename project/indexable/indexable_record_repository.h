//
// Created by arthu on 08/06/2026.
//

#ifndef INDEXABLE_RECORD_REPOSITORY_H
#define INDEXABLE_RECORD_REPOSITORY_H
#include "../services/file_repository.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @struct IndexableRecord
 * @brief Represents an indexable subway record used in AVL indexing.
 *
 * This structure stores the essential information needed to index and
 * retrieve subway records efficiently. Each record contains:
 * - The relative record number (RRN), which uniquely identifies the record
 *   within the database.
 * - The station ID, which serves as the key for indexing in the AVL tree.
 */
struct IndexableRecord {
    uint32_t rrn;       /**< Relative Record Number - unique identifier in the database */
    uint32_t stationID; /**< Station ID used as the key in the AVL index */
};


bool IndexableRecordRepository_writeRecord(struct DataFile *dataFile, const struct IndexableRecord *record);
bool IndexableRecordRepository_readRecord(struct DataFile *dataFile, struct IndexableRecord **record);

#endif //INDEXABLE_RECORD_REPOSITORY_H
