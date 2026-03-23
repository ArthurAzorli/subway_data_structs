#include "data_base_repository.h"
#include "../core/file/file_repository.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRASH 0x24
#define HEADER_LENGTH 17
#define RECORD_LENGTH 80
#define RECORD_STATUS_LENGTH 5


struct DataBase {
    uint32_t lastRemoved;
    uint32_t nextInsert;
    uint32_t stationsCount;
    uint32_t pairStationsCount;
    struct DataFile *dataFile;
};

bool DataBaseRepository_isDataBaseValid(const struct DataBase *dataBase) {
    if (dataBase == NULL || dataBase->dataFile == NULL) {
        printf("ERROR: Invalid data base\n");
        return false;
    }
    return true;
}

bool DataBaseRepository_isRecordValid(const struct SubwayLineRecord *record) {
    if (record == NULL) {
        printf("ERROR: Ivalid Subway line record\n");
        return false;
    }
    if (record->originStationID == EMPTY) {
        printf("ERROR: Ivalid Subway line record (Origin Station ID is Empty)\n");
        return false;
    }
    if (record->stationName == NULL || record->stationNameLength == 0) {
        printf("ERROR: Ivalid Subway line record (Station Name is Empty)\n");
        return false;
    }
    return true;
}

size_t DataBaseRepository_getByteOffsetFromRRN(const size_t rrn) {
    return HEADER_LENGTH + (RECORD_LENGTH * rrn);
}

bool DataBaseRepository_writeHeader(const struct DataBase *dataBase) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, dataBase->lastRemoved)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, dataBase->nextInsert)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, dataBase->stationsCount)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, dataBase->pairStationsCount)) return false;
    return true;
}

bool DataBaseRepository_readHeader(struct DataBase *dataBase) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &dataBase->lastRemoved)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &dataBase->nextInsert)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &dataBase->stationsCount)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &dataBase->pairStationsCount)) return false;
    return true;
}

bool DataBaseRepository_writeRecordData(const struct DataBase *dataBase, struct SubwayLineRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->originStationID)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->originLineID)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->destinationStationID)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->destinationLineID)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->interactionLineID)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->interactionStationID)) return false;

    size_t remaining = RECORD_LENGTH - (10 * UINT32_BYTES_COUNT + 1);
    record->stationNameLength = remaining > record->stationNameLength ? record->stationNameLength : remaining;
    remaining -= record->stationNameLength;
    record->lineNameLength = remaining > record->lineNameLength ? record->lineNameLength : remaining;

    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->stationNameLength)) return false;
    if (record->stationNameLength > 0 && record->stationNameLength != EMPTY) {
        if (!FileRepository_writeString(dataBase->dataFile, record->stationNameLength, record->stationName))
            return
                    false;
    }
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->lineNameLength)) return false;
    if (record->lineNameLength > 0 && record->lineNameLength != EMPTY) {
        if (!FileRepository_writeString(dataBase->dataFile, record->lineNameLength, record->lineName)) return false;
    }

    while (remaining > 0) {
        FileRepository_writeByte(dataBase->dataFile, TRASH);
        remaining--;
    }
    return FileRepository_flush(dataBase->dataFile);;
}

bool DataBaseRepository_readRecordData(const struct DataBase *dataBase, struct SubwayLineRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (record == NULL) {
        printf("ERROR: Failed to allocate data record\n");
        return false;
    }

    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->originStationID)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->originLineID)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->destinationStationID)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->destinationLineID)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->interactionLineID)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->interactionStationID)) return false;

    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, (uint32_t *) &record->stationNameLength)) return false;
    record->stationName = malloc(record->stationNameLength + 1);
    if (record->stationName == NULL) {
        printf("ERROR: Failed to allocate record station name\n");
        return false;
    }

    if (!FileRepository_readString(dataBase->dataFile, record->stationNameLength, record->stationName)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, (uint32_t *) &record->stationNameLength)) return false;
    record->lineName = malloc(record->lineNameLength + 1);
    if (record->lineName == NULL) {
        printf("ERROR: Failed to allocate record line name\n");
        return false;
    }
    return true;
}

bool DataBaseRepository_affectsStationsPairs(struct DataBase *dataBase, struct SubwayLineRecord *record) {
    //TODO: verify if has a new Statioins Pair
    return false;
}


struct DataBase *DataBaseRepository_init(const String path) {
    struct DataFile *dataFile = FileRepository_openOrCreate(path);
    if (dataFile == NULL) {
        printf("ERROR: Failed to open data base file\n");
        return NULL;
    }

    struct DataBase *database = malloc(sizeof(struct DataBase));
    if (database == NULL) {
        printf("ERROR: Failed to allocate data base\n");
        return NULL;
    }
    database->dataFile = dataFile;
    const size_t fileSize = FileRepository_fileSize(dataFile);
    if (fileSize < HEADER_LENGTH) {
        database->lastRemoved = EMPTY;
        database->nextInsert = 0;
        database->stationsCount = 0;
        database->pairStationsCount = 0;
        if (!DataBaseRepository_writeHeader(database)) {
            printf("ERROR: Failed to write to data header file\n");
            FileRepository_close(dataFile);
            free(database);
            return NULL;
        }
    } else if (!DataBaseRepository_readHeader(database)) {
        printf("ERROR: Failed to load to data header file\n");
        FileRepository_close(dataFile);
        free(database);
        return NULL;
    }
    return database;
}

bool DataBaseRepository_createRecord(struct DataBase *dataBase, struct SubwayLineRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRecordValid(record)) return false;

    uint32_t rrn = dataBase->nextInsert;
    if (dataBase->lastRemoved != EMPTY) {
        uint32_t nextRemoved;
        if (!FileRepository_goTo(dataBase->dataFile, dataBase->nextInsert + 1)) return false;
        if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &nextRemoved)) return false;
        rrn = dataBase->lastRemoved;
        dataBase->lastRemoved = nextRemoved;
    }

    record->rrn = rrn;
    const size_t byteOffset = DataBaseRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataBase->dataFile, byteOffset)) return false;
    if (!FileRepository_writeBool(dataBase->dataFile, false)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, EMPTY)) return false;
    if (!DataBaseRepository_writeRecordData(dataBase, record)) return false;

    dataBase->nextInsert++;
    dataBase->stationsCount++;
    if (DataBaseRepository_affectsStationsPairs(dataBase, record)) dataBase->pairStationsCount++;
    if (!DataBaseRepository_writeHeader(dataBase)) return false;
    return FileRepository_flush(dataBase->dataFile);
}

struct SubwayLineRecord *DataBaseRepository_readRecord(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return NULL;
    if (rrn >= dataBase->nextInsert) {
        printf("ERROR: RRN %zu is out of bounds\n", rrn);
        return NULL;
    }

    const size_t byteOffset = DataBaseRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataBase->dataFile, byteOffset)) return NULL;

    bool isRemoved;
    if (!FileRepository_readBool(dataBase->dataFile, &isRemoved)) return NULL;
    if (isRemoved) return NULL;


    struct SubwayLineRecord* record = malloc(sizeof(struct SubwayLineRecord));
    if (record == NULL) {
        printf("ERROR: Failed to allocate data record\n");
        return NULL;
    }
    record->rrn = rrn;
    if (!DataBaseRepository_readRecordData(dataBase, record)) {
        printf("ERROR: Failed to read record data file\n");
        free(record);
        return NULL;
    }
    return record;
}

bool DataBaseRepository_deleteRecord(struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (rrn >= dataBase->nextInsert) {
        printf("ERROR: RRN %zu is out of bounds\n", rrn);
        return false;
    }

    const size_t byteOffset = DataBaseRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataBase->dataFile, byteOffset)) return false;

    bool isRemoved;
    if (!FileRepository_readBool(dataBase->dataFile, &isRemoved)) return false;
    if (!FileRepository_move(dataBase->dataFile, -1)) return false;
    if (isRemoved) return true;

    isRemoved = true;
    if (!FileRepository_writeBool(dataBase->dataFile, isRemoved)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, dataBase->lastRemoved)) return false;
    struct SubwayLineRecord record;
    if (!DataBaseRepository_readRecordData(dataBase, &record))return false;

    dataBase->lastRemoved = rrn;
    dataBase->stationsCount--;
    if (DataBaseRepository_affectsStationsPairs(dataBase, &record)) dataBase->pairStationsCount--;
    if (!DataBaseRepository_writeHeader(dataBase)) return false;
    return FileRepository_flush(dataBase->dataFile);
}

bool DataBaseRepository_existRecord(const struct DataBase* dataBase, const String stationName) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;

    for (size_t rrn = 0; rrn < dataBase->nextInsert; rrn++) {
        struct SubwayLineRecord* record = DataBaseRepository_readRecord(dataBase, rrn);
        if (record == NULL || record->stationName == NULL) continue;
        if (strcmp(record->stationName, stationName) == 0) {
            free(record);
            return true;
        }
        free(record);
    }
    return false;
}
