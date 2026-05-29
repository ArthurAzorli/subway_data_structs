#include "subway_header_repository.h"
#include "../../services/file/file_repository.h"
#include "../../services/utils/types.h"
#include <stdlib.h>

#define HEADER_FILE_LENGTH 16
#define HEADER_FILE_SECTION_OFFSET 0


bool SubwayHeaderRepository_isHeaderValid(const struct DataSubwayHeader *header) {
    if (header == NULL) return false;
    if (header->pairStationsCount == EMPTY) return false;
    if (header->stationsCount == EMPTY) return false;
    if (header->nextInsert == EMPTY) return false;
    return true;
}

bool SubwayHeaderRepository_writeHeader(const struct DataSubwayHeader *header, struct DataFile *dataFile) {
    if (header == NULL || dataFile == NULL) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->lastRemoved)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->nextInsert)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->stationsCount)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->pairStationsCount)) return false;
    return true;
}

bool SubwayHeaderRepository_readHeader(struct DataSubwayHeader *header, struct DataFile *dataFile) {
    if (dataFile == NULL) return false;
    if (!FileRepository_read(dataFile, INTEGER, &header->lastRemoved)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &header->nextInsert)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &header->stationsCount)) return false;
    if (!FileRepository_read(dataFile, INTEGER, &header->pairStationsCount)) return false;
    return true;
}

struct DataSubwayHeader *SubwayHeaderRepository_init(struct DataFile *dataFile) {
    if (dataFile == NULL)return NULL;

    if (!FileRepository_goto(dataFile, HEADER_FILE_SECTION_OFFSET)) return NULL;
    struct DataSubwayHeader *header = malloc(sizeof(struct DataSubwayHeader));
    if (header == NULL) return NULL;

    header->lastRemoved = EMPTY;
    header->nextInsert = 0;
    header->stationsCount = 0;
    header->pairStationsCount = 0;

    // If there is no header or an incomplete one, reset it, otherwise read the previous header
    const size_t fileSize = FileRepository_fileSize(dataFile);
    if (fileSize < HEADER_FILE_LENGTH) {
        if (!SubwayHeaderRepository_save(header, dataFile)) {
            free(header);
            return NULL;
        }
    } else {
        if (!SubwayHeaderRepository_readHeader(header, dataFile)) {
            free(header);
            return NULL;
        }
    }
    return header;
}

bool SubwayHeaderRepository_save(const struct DataSubwayHeader *header, struct DataFile *dataFile) {
    if (!SubwayHeaderRepository_isHeaderValid(header) || dataFile == NULL) return false;
    if (!FileRepository_goto(dataFile, HEADER_FILE_SECTION_OFFSET)) return false;
    if (!SubwayHeaderRepository_writeHeader(header, dataFile)) return false;
    return true;
}
