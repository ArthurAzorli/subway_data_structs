//
// Created by User on 28/03/2026.
//

#include "header_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define HEADER_FILE_LENGTH 16
#define HEADER_FILE_SECTION_OFFSET 0

bool HeaderRepository_isHeaderValid(const struct DataHeader *header) {
    if (header == NULL) return false;
    if (header->pairStationsCount == EMPTY) return false;
    if (header->stationsCount == EMPTY) return false;
    if (header->nextInsert == EMPTY) return false;
    return true;
}

bool HeaderRepository_writeHeader(const struct DataHeader *header, struct DataFile *dataFile) {
    if (header == NULL || dataFile == NULL) return false;
    if (!FileRepository_writeInt(dataFile, header->lastRemoved)) return false;
    if (!FileRepository_writeInt(dataFile, header->nextInsert)) return false;
    if (!FileRepository_writeInt(dataFile, header->stationsCount)) return false;
    if (!FileRepository_writeInt(dataFile, header->pairStationsCount)) return false;
    return true;
}

bool HeaderRepository_readHeader(struct DataHeader *header, struct DataFile *dataFile) {
    if (dataFile == NULL) return false;
    if (!FileRepository_readInt(dataFile, &header->lastRemoved)) return false;
    if (!FileRepository_readInt(dataFile, &header->nextInsert)) return false;
    if (!FileRepository_readInt(dataFile, &header->stationsCount)) return false;
    if (!FileRepository_readInt(dataFile, &header->pairStationsCount)) return false;
    return true;
}


struct DataHeader *HeaderRepository_init(struct DataFile *dataFile) {
    if (dataFile == NULL) {
        printf("ERROR: Invalid data file\n");
        return NULL;
    }

    if (!FileRepository_goTo(dataFile, HEADER_FILE_SECTION_OFFSET)) return NULL;
    struct DataHeader *header = malloc(sizeof(struct DataHeader));
    if (header == NULL) {
        printf("ERROR: Could not allocate memory for the header\n");
        return NULL;
    }

    header->lastRemoved = EMPTY;
    header->nextInsert = 0;
    header->stationsCount = 0;
    header->pairStationsCount = 0;

    const size_t fileSize = FileRepository_fileSize(dataFile);
    if (fileSize < HEADER_FILE_LENGTH) {
        if (!HeaderRepository_save(header, dataFile)) {
            free(header);
            return NULL;
        }
    } else {
        if (!HeaderRepository_readHeader(header, dataFile)) {
            free(header);
            return NULL;
        }
    }
    return header;
}

bool HeaderRepository_save(const struct DataHeader *header, struct DataFile *dataFile) {
    if (!HeaderRepository_isHeaderValid(header)) {
        printf("ERROR: Invalid header file\n");
        return false;
    }
    if (dataFile == NULL) {
        printf("ERROR: Invalid data file\n");
        return false;
    }
    if (!FileRepository_goTo(dataFile, HEADER_FILE_SECTION_OFFSET)) return false;
    if (!HeaderRepository_writeHeader(header, dataFile)) {
        printf("ERROR: Could not write header\n");
        return false;
    }
    return true;
}
