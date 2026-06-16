#include "indexable_record_repository.h"

#include "../services/file_repository.h"
#include <stdlib.h>

/**
 * @brief Writes an indexable record to a data file.
 *
 * Stores the station ID and relative record number (RRN) of the given record
 * into the binary file using the FileRepository interface.
 *
 * @param dataFile Pointer to the DataFile structure representing the open file
 * @param record Pointer to the IndexableRecord to write
 * @return true if the record was successfully written, false otherwise
 */
bool IndexableRecordRepository_writeRecord(struct DataFile *dataFile, const struct IndexableRecord *record) {
    if (dataFile == NULL || record == NULL) return false;

    if (!FileRepository_write(dataFile, INTEGER, &record->stationID, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &record->rrn, 1)) return false;
    return true;
}

/**
 * @brief Reads an indexable record from a data file.
 *
 * Allocates memory for a new IndexableRecord and reads its station ID and RRN
 * from the binary file using the FileRepository interface.
 *
 * @param dataFile Pointer to the DataFile structure representing the open file
 * @param record Double pointer to store the newly allocated IndexableRecord
 * @return true if the record was successfully read, false otherwise
 *
 * @note The caller is responsible for freeing the allocated record.
 */
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
