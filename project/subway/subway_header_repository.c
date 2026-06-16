#include "subway_header_repository.h"
#include "../services/file_repository.h"
#include "../services/types.h"
#include <stdlib.h>

struct DataSubwayHeader * SubwayHeaderRepository_init() {
    struct DataSubwayHeader *header = malloc(sizeof(struct DataSubwayHeader));
    if (header == NULL) return NULL;

    header->lastRemoved = EMPTY;
    header->nextInsert = 0;
    header->stationsCount = 0;
    header->pairStationsCount = 0;

    return header;
}


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

bool SubwayHeaderRepository_write(const struct DataSubwayHeader *header, struct DataFile *dataFile) {
    //validar se é valido
    if (header == NULL) return false;
    if (header->pairStationsCount == EMPTY) return false;
    if (header->stationsCount == EMPTY) return false;
    if (header->nextInsert == EMPTY) return false;

    //escreve o header
    if (!FileRepository_write(dataFile, INTEGER, &header->lastRemoved, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->nextInsert, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->stationsCount, 1)) return false;
    if (!FileRepository_write(dataFile, INTEGER, &header->pairStationsCount, 1)) return false;
    return true;
}
