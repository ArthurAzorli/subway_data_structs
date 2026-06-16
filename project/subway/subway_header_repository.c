#include "subway_header_repository.h"
#include "../services/file_repository.h"
#include "../services/types.h"
#include <stdlib.h>

/**
 * @brief Initializes a new subway header structure.
 *
 * Allocates memory for a DataSubwayHeader and sets default values:
 * - lastRemoved = EMPTY
 * - nextInsert = 0
 * - stationsCount = 0
 * - pairStationsCount = 0
 *
 * @return Pointer to the newly allocated DataSubwayHeader, or NULL if allocation fails
 */
struct DataSubwayHeader * SubwayHeaderRepository_init() {
    struct DataSubwayHeader *header = malloc(sizeof(struct DataSubwayHeader));
    if (header == NULL) return NULL;

    header->lastRemoved = EMPTY;
    header->nextInsert = 0;
    header->stationsCount = 0;
    header->pairStationsCount = 0;

    return header;
}

/**
 * @brief Reads the subway header from a data file.
 *
 * Reads the header fields (lastRemoved, nextInsert, stationsCount, pairStationsCount)
 * from the given DataFile. If any read operation fails, the header is freed and NULL is returned.
 *
 * @param dataFile Pointer to the DataFile structure representing the open file
 * @return Pointer to the DataSubwayHeader read from the file, or NULL if reading fails
 */
struct DataSubwayHeader *SubwayHeaderRepository_read(struct DataFile *dataFile) {
    if (dataFile == NULL) return NULL;

    struct DataSubwayHeader *header = SubwayHeaderRepository_init();

    if (!FileRepository_read(dataFile, INTEGER, &header->lastRemoved, 1)) {
        free(header);
        return NULL;
    }
    if (!FileRepository_read(dataFile, INTEGER, &header->nextInsert, 1)) {
        free(header);
        return NULL;
    }
    if (!FileRepository_read(dataFile, INTEGER, &header->stationsCount, 1)) {
        free(header);
        return NULL;
    }
    if (!FileRepository_read(dataFile, INTEGER, &header->pairStationsCount, 1)) {
        free(header);
        return NULL;
    }

    return header;
}

/**
 * @brief Writes the subway header to a data file.
 *
 * Validates the header fields before writing. If any of the fields are equal to EMPTY,
 * the operation fails. Otherwise, writes all header fields to the file.
 *
 * @param header Pointer to the DataSubwayHeader to write
 * @param dataFile Pointer to the DataFile structure representing the open file
 * @return true if the header was successfully written, false otherwise
 */
bool SubwayHeaderRepository_write(const struct DataSubwayHeader *header, struct DataFile *dataFile) {
    //validate header fields
    if (header == NULL) return false;
    if (header->pairStationsCount == EMPTY) return false;
    if (header->stationsCount == EMPTY) return false;
    if (header->nextInsert == EMPTY) return false;

    //write header fields
    if (!FileRepository_write(dataFile, INTEGER, &header->lastRemoved, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->nextInsert, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->stationsCount, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->pairStationsCount, 1)) return false;
    return true;
}
