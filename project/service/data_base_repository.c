#include "data_base_repository.h"

#include <stdio.h>
#include <stdlib.h>

#include "../core/bytes/byte_stream.h"

#define TRASH 0x24
#define HEADER_LENGTH 17
#define RECORD_LENGTH 80
#define CREATE_MODE "w"
#define EDIT_MODE "r+b"

struct Header {
    size_t lastRemoved;
    size_t nextInsert;
    size_t stationsCount;
    size_t pairStationsCount;
};

void DataBaseRepository_writeHeader(FILE* file, struct Header) {
    fseek(file, 0, SEEK_SET);

}



bool DataBaseRepository_init(String path) {

    struct DataBaseRepository *repository = malloc(sizeof(struct DataBaseRepository));
    repository->path = path;
    FILE *file = fopen(path, EDIT_MODE);

    if (file != NULL) {
        bool consistent;

        fclose(file);
        if (consistent) return true;
    }

    file = fopen(path, CREATE_MODE);
    bool consistent = false;





    fwrite()



    fread(&repository->consistent, UINT8_BYTES_COUNT, 1, file);
    fread(&repository->lastRemoved, UINT32_BYTES_COUNT, 1, file);
    fread(&repository->nextRecord, UINT32_BYTES_COUNT, 1, file);
    fread(&repository->stationsCount, UINT32_BYTES_COUNT, 1, file);
    fread(&repository->pairStationsCount, UINT32_BYTES_COUNT, 1, file);

    fclose(file);



    repository->lastRemoved = EMPTY;
    repository->nextRecord = 0;
    repository->stationsCount = 0;
    repository->pairStationsCount = 0;

}
