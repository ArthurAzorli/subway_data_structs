#include "data_base_repository.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/file/file_repository.h"

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
        printf("ERROR: Invalid Subway line record\n");
        return false;
    }
    if (record->originStationID == EMPTY) {
        printf("ERROR: Invalid Subway line record (Origin Station ID is Empty)\n");
        return false;
    }
    if (record->stationName == NULL || record->stationNameLength == 0) {
        printf("ERROR: Invalid Subway line record (Station Name is Empty)\n");
        return false;
    }
    return true;
}

size_t DataBaseRepository_getByteOffsetFromRRN(const size_t rrn) {
    return HEADER_LENGTH + RECORD_LENGTH * rrn;
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
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->destinationDistant)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->interactionLineID)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->interactionStationID)) return false;

    size_t remaining = RECORD_LENGTH - (8 * UINT32_BYTES_COUNT + RECORD_STATUS_LENGTH);
    record->stationNameLength = remaining > record->stationNameLength ? record->stationNameLength : remaining;
    remaining -= record->stationNameLength;
    record->lineNameLength = remaining > record->lineNameLength ? record->lineNameLength : remaining;

    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->stationNameLength)) return false;
    if (record->stationNameLength > 0 && record->stationNameLength != EMPTY) {
        if (!FileRepository_writeString(dataBase->dataFile, record->stationNameLength, record->stationName))
            return false;
    }
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, record->lineNameLength)) return false;
    if (record->lineNameLength > 0 && record->lineNameLength != EMPTY) {
        if (!FileRepository_writeString(dataBase->dataFile, record->lineNameLength, record->lineName)) return false;
    }

    while (remaining > 0) {
        FileRepository_writeByte(dataBase->dataFile, TRASH);
        remaining--;
    }
    return FileRepository_flush(dataBase->dataFile);
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
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->destinationDistant)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->interactionLineID)) return false;
    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->interactionStationID)) return false;

    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->stationNameLength)) return false;
    record->stationName = malloc(record->stationNameLength + 1);
    if (record->stationName == NULL) {
        printf("ERROR: Failed to allocate record station name\n");
        return false;
    }
    if (record->stationNameLength > 0) {
        if (!FileRepository_readString(dataBase->dataFile, record->stationNameLength, record->stationName)) return false;
    }
    record->stationName[record->stationNameLength] = '\0';

    if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &record->lineNameLength)) return false;
    record->lineName = malloc(record->lineNameLength + 1);
    if (record->lineName == NULL) {
        printf("ERROR: Failed to allocate record line name\n");
        return false;
    }
    if (record->lineNameLength > 0) {
        if (!FileRepository_readString(dataBase->dataFile, record->lineNameLength, record->lineName)) return false;
    }
    record->lineName[record->lineNameLength] = '\0';
    return true;
}


bool DataBaseRepository_affectsStationsPairs(const struct DataBase *dataBase, const struct SubwayLineRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRecordValid(record)) return false;
    if (record->destinationStationID == EMPTY) return false;

    size_t counter = 0;
    for (size_t rrn = 0; rrn < dataBase->nextInsert; rrn++) {
        struct SubwayLineRecord *other = DataBaseRepository_readRecord(dataBase, rrn);
        if (other == NULL) continue;
        if (DataBaseRepository_isRecordValid(other)) {
            if (record->destinationStationID == other->originStationID || record->originStationID == other->
                destinationStationID)
                counter++;
        }
        SubwayLineRecord_free(other);
    }

    return counter == 2;
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
    database->lastRemoved = EMPTY;
    database->nextInsert = 0;
    database->stationsCount = 0;
    database->pairStationsCount = 0;

    const size_t fileSize = FileRepository_fileSize(dataFile);
    if (fileSize < HEADER_LENGTH) {
        if (!DataBaseRepository_writeHeader(database)) {
            printf("ERROR: Failed to write to data header file\n");
            DataBaseRepository_close(database);
            return NULL;
        }
    } else if (!DataBaseRepository_readHeader(database)) {
        printf("ERROR: Failed to load to data header file\n");
        DataBaseRepository_close(database);
        return NULL;
    }
    return database;
}

bool DataBaseRepository_createRecord(struct DataBase *dataBase, struct SubwayLineRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRecordValid(record)) return false;

    bool reuse = false;
    uint32_t rrn = dataBase->nextInsert;
    if (dataBase->lastRemoved != EMPTY) {
        rrn = dataBase->lastRemoved;
        const size_t byteOffset = DataBaseRepository_getByteOffsetFromRRN(rrn);
        if (!FileRepository_goTo(dataBase->dataFile, byteOffset + 1)) return false;
        if (!FileRepository_readInt(dataBase->dataFile, BIG_ENDIAN, &dataBase->lastRemoved)) return false;
        reuse = true;
    }

    record->rrn = rrn;
    const size_t byteOffset = DataBaseRepository_getByteOffsetFromRRN(rrn);
    if (!FileRepository_goTo(dataBase->dataFile, byteOffset)) return false;
    if (!FileRepository_writeBool(dataBase->dataFile, false)) return false;
    if (!FileRepository_writeInt(dataBase->dataFile, BIG_ENDIAN, EMPTY)) return false;
    if (!DataBaseRepository_writeRecordData(dataBase, record)) return false;

    if (!reuse) dataBase->nextInsert++;
    dataBase->stationsCount++;
    if (DataBaseRepository_affectsStationsPairs(dataBase, record)) dataBase->pairStationsCount++;
    if (!FileRepository_goTo(dataBase->dataFile, 1)) return false;
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


    struct SubwayLineRecord *record = SubwayLineRecord_init();
    if (record == NULL) return NULL;

    record->rrn = rrn;
    if (!FileRepository_move(dataBase->dataFile, UINT32_BYTES_COUNT)) return NULL;
    if (!DataBaseRepository_readRecordData(dataBase, record)) {
        printf("ERROR: Failed to read record data file\n");
        free(record);
        return NULL;
    }

    return record;
}

bool DataBaseRepository_updateRecord(struct DataBase *dataBase, struct SubwayLineRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRecordValid(record)) return false;
    if (record->rrn >= dataBase->nextInsert) {
        printf("ERROR: RRN %zu is out of bounds\n", record->rrn);
        return false;
    }

    struct SubwayLineRecord *oldRecord = DataBaseRepository_readRecord(dataBase, record->rrn);
    if (oldRecord == NULL) return false;

    bool changeHeader = false;
    if ((oldRecord->originStationID != record->originStationID || oldRecord->destinationStationID != record->
         destinationStationID) && DataBaseRepository_affectsStationsPairs(dataBase, record)) {
        if (oldRecord->destinationStationID == EMPTY) {
            dataBase->pairStationsCount++;
            changeHeader = true;
        } else if (record->destinationStationID == EMPTY) {
            dataBase->pairStationsCount--;
            changeHeader = true;
        }
    }

    bool changeRecord = false;
    if (oldRecord->originStationID != record->originStationID) changeRecord = true;
    if (oldRecord->originLineID != record->originLineID) changeRecord = true;
    if (oldRecord->destinationStationID != record->destinationStationID) changeRecord = true;
    if (oldRecord->destinationDistant != record->destinationDistant) changeRecord = true;
    if (oldRecord->interactionStationID != record->interactionStationID) changeRecord = true;
    if (oldRecord->interactionLineID != record->interactionLineID) changeRecord = true;
    if (oldRecord->stationNameLength != record->stationNameLength) changeRecord = true;
    if (oldRecord->lineNameLength != record->lineNameLength) changeRecord = true;
    if (strcmp(oldRecord->stationName, record->stationName) != 0) changeRecord = true;

    if (oldRecord->lineName != NULL && record->lineName != NULL) {
        if (strcmp(oldRecord->lineName, record->lineName) != 0) changeRecord = true;
    } else if (oldRecord->lineName != record->lineName) {
        changeRecord = true;
    }

    if (!changeRecord) {
        SubwayLineRecord_free(oldRecord);
        return false;
    }

    if (changeHeader) {
        if (!FileRepository_goTo(dataBase->dataFile, 1)) {
            SubwayLineRecord_free(oldRecord);
            return false;
        }

        if (!DataBaseRepository_writeHeader(dataBase)) {
            SubwayLineRecord_free(oldRecord);
            return false;
        }
    }

    const size_t byteOffset = DataBaseRepository_getByteOffsetFromRRN(record->rrn);
    if (!FileRepository_goTo(dataBase->dataFile, byteOffset + RECORD_STATUS_LENGTH)) {
        SubwayLineRecord_free(oldRecord);
        return false;
    }
    if (!DataBaseRepository_writeRecordData(dataBase, record)) {
        SubwayLineRecord_free(oldRecord);
        return false;
    }

    SubwayLineRecord_free(oldRecord);
    return FileRepository_flush(dataBase->dataFile);
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

    struct SubwayLineRecord *record = SubwayLineRecord_init();
    if (record == NULL) return false;
    if (!DataBaseRepository_readRecordData(dataBase, record)) {
        SubwayLineRecord_free(record);
        return false;
    }

    dataBase->lastRemoved = rrn;
    dataBase->stationsCount--;
    if (DataBaseRepository_affectsStationsPairs(dataBase, record)) dataBase->pairStationsCount--;
    if (!FileRepository_goTo(dataBase->dataFile, 1)) {
        SubwayLineRecord_free(record);
        return false;
    }
    if (!DataBaseRepository_writeHeader(dataBase)) {
        SubwayLineRecord_free(record);
        return false;
    }
    SubwayLineRecord_free(record);
    return FileRepository_flush(dataBase->dataFile);
}

bool DataBaseRepository_existRecord(const struct DataBase *dataBase, const String stationName) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (stationName == NULL) {
        printf("ERROR: Invalid station name\n");
        return false;
    }

    for (size_t rrn = 0; rrn < dataBase->nextInsert; rrn++) {
        struct SubwayLineRecord *record = DataBaseRepository_readRecord(dataBase, rrn);
        if (record == NULL) continue;
        if (record->stationName != NULL && strcmp(record->stationName, stationName) == 0) {
            SubwayLineRecord_free(record);
            return true;
        }
        SubwayLineRecord_free(record);
    }
    return false;
}

void DataBaseRepository_close(struct DataBase *dataBase) {
    if (dataBase == NULL) return;
    if (dataBase->dataFile != NULL) FileRepository_close(dataBase->dataFile);
    free(dataBase);
}
