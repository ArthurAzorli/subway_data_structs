#include "data_base_repository.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header_repository.h"
#include "../core/file/file_repository.h"
#include "record_repository.h"

bool DataBaseRepository_isDataBaseValid(const struct DataBase *dataBase) {
    if (dataBase == NULL || dataBase->dataFile == NULL || dataBase->dataHeader == NULL) {
        printf("ERROR: Invalid data base\n");
        return false;
    }
    return true;
}

bool DataBaseRepository_isRRNValid(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    return dataBase->dataHeader->nextInsert > rrn;
}

size_t DataBaseRepository_countStation(const struct DataBase *dataBase, const size_t stationNameLength, const String stationName) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return 0;
    if (stationName == NULL) {
        printf("ERROR: Invalid station name\n");
        return 0;
    }

    size_t count = 0;
    for (size_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
        struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
        if (record == NULL || record->stationName == NULL) continue;
        if (record->stationNameLength == stationNameLength && memcmp(record->stationName, stationName, stationNameLength) == 0) {
            count++;
        }
        SubwayRecord_free(record);
    }
    return count;
}

size_t DatabaseRepository_countStationsPairs(const struct DataBase *dataBase, const struct SubwayRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase) || record == NULL) return 0;
    if (record->originStationID == EMPTY || record->destinationStationID == EMPTY) return 0;

    size_t count = 0;
    for (size_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
        struct SubwayRecord *other = RecordRepository_readRecord(dataBase->dataFile, rrn);
        if (other == NULL) continue;

        if (record->originStationID == other->originStationID &&
            record->destinationStationID == other->destinationStationID) count++;
        SubwayRecord_free(other);
    }
    return count;
}

struct DataBase *DataBaseRepository_init(const String path) {
    struct DataFile *dataFile = FileRepository_openOrCreate(path);
    if (dataFile == NULL) {
        printf("ERROR: Failed to open data base file\n");
        return NULL;
    }

    struct DataHeader *dataHeader = HeaderRepository_init(dataFile);
    if (dataHeader == NULL) {
        printf("ERROR: Failed to initialize data header\n");
        FileRepository_close(dataFile);
        return NULL;
    }

    struct DataBase *database = malloc(sizeof(struct DataBase));
    if (database == NULL) {
        printf("ERROR: Failed to allocate memory for data base\n");
        FileRepository_close(dataFile);
        free(dataHeader);
        return NULL;
    }

    database->dataHeader = dataHeader;
    database->dataFile = dataFile;

    return database;
}

bool DataBaseRepository_createRecord(const struct DataBase *dataBase, struct SubwayRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;

    bool reuse = false;
    uint32_t rrn = dataBase->dataHeader->nextInsert;
    const uint32_t lastRemoved = dataBase->dataHeader->lastRemoved;
    if (lastRemoved != EMPTY) {
        rrn = lastRemoved;
        if (!RecordRepository_readLastRemoved(dataBase->dataFile, lastRemoved, &dataBase->dataHeader->lastRemoved)) {
            printf("ERROR: Failed to read last removed record\n");
            return false;
        }
        reuse = true;
    }

    record->rrn = rrn;
    if (!RecordRepository_writeRecord(dataBase->dataFile, record)) {
        printf("ERROR: Failed to write record data file\n");
        return false;
    }

    if (!reuse) dataBase->dataHeader->nextInsert++;
    if (DataBaseRepository_countStation(dataBase, record->stationNameLength, record->stationName) == 1) dataBase->dataHeader->stationsCount++;
    if (record->destinationStationID != EMPTY && DatabaseRepository_countStationsPairs(dataBase, record) == 1) dataBase->dataHeader->pairStationsCount++;

    if (!HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile)) {
        printf("ERROR: Failed to write to data header file\n");
        return false;
    }
    return FileRepository_flush(dataBase->dataFile);
}

struct SubwayRecord *DataBaseRepository_readRecord(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return NULL;
    if (!DataBaseRepository_isRRNValid(dataBase, rrn)) {
        printf("ERROR: RRN %zu is out of bounds\n", rrn);
        return NULL;
    }

    struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
    if (record == NULL) {
        printf("ERROR: Failed to read record data file\n");
        return NULL;
    }

    return record;
}

bool DataBaseRepository_updateRecord(const struct DataBase *dataBase, struct SubwayRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRRNValid(dataBase, record->rrn)) return false;


    struct SubwayRecord *oldRecord = RecordRepository_readRecord(dataBase->dataFile, record->rrn);
    if (oldRecord == NULL) {
        printf("ERROR: Failed to read record data file\n");
        return false;
    }

    if (SubwayRecord_isEquals(oldRecord, record)) {
        SubwayRecord_free(oldRecord);
        return false;
    }

    bool changeHeader = false;
    if (strcmp(oldRecord->stationName, record->stationName) != 0) {
        if (DataBaseRepository_countStation(dataBase, oldRecord->stationNameLength, oldRecord->stationName) == 1) {
            dataBase->dataHeader->stationsCount--;
            changeHeader = true;
        }
        if (DataBaseRepository_countStation(dataBase, record->stationNameLength, record->stationName) == 0) {
            dataBase->dataHeader->stationsCount++;
            changeHeader = true;
        }
    }

    if (oldRecord->originStationID != record->originStationID || oldRecord->destinationStationID != record->destinationStationID) {
        if (DatabaseRepository_countStationsPairs(dataBase, oldRecord) == 1) {
            dataBase->dataHeader->pairStationsCount--;
            changeHeader = true;
        }

        if (DatabaseRepository_countStationsPairs(dataBase, record) == 0) {
            dataBase->dataHeader->pairStationsCount++;
            changeHeader = true;
        }
    }

    if (changeHeader) {
        if (!HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile)) {
            printf("ERROR: Failed to save header data file\n");
            SubwayRecord_free(oldRecord);
            return false;
        }
    }

    if (!RecordRepository_writeRecord(dataBase->dataFile, record)) {
        printf("ERROR: Failed to write record data file\n");
        SubwayRecord_free(oldRecord);
        return false;
    }

    SubwayRecord_free(oldRecord);
    return FileRepository_flush(dataBase->dataFile);
}

bool DataBaseRepository_deleteRecord(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRRNValid(dataBase, rrn)) {
        printf("ERROR: RRN %zu is out of bounds\n", rrn);
        return false;
    }


    struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
    if (record == NULL) {
        printf("ERROR: Record %zu already removed or invalid\n", rrn);
        return false;
    }

    if (!RecordRepository_removeRecord(dataBase->dataFile, rrn, &dataBase->dataHeader->lastRemoved)) {
        printf("ERROR: Failed to remove record data file\n");
        return false;
    }

    if (DataBaseRepository_countStation(dataBase, record->stationNameLength, record->stationName) == 0) dataBase->dataHeader->stationsCount--;
    if (DatabaseRepository_countStationsPairs(dataBase, record) == 0) dataBase->dataHeader->pairStationsCount--;
    if (!HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile)) {
        printf("ERROR: Failed to write to data header file\n");
        return false;
    }
    SubwayRecord_free(record);
    return FileRepository_flush(dataBase->dataFile);
}

bool DataBaseRepository_existRecord(const struct DataBase *dataBase, const String stationName) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (stationName == NULL) {
        printf("ERROR: Invalid station name\n");
        return false;
    }

    for (size_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
        struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
        if (record == NULL || record->stationName == NULL) continue;
        if (memcmp(record->stationName, stationName, record->stationNameLength) == 0) {
            SubwayRecord_free(record);
            return true;
        }
        SubwayRecord_free(record);
    }
    return false;
}

void DataBaseRepository_close(struct DataBase *dataBase) {
    if (dataBase == NULL) return;
    if (dataBase->dataHeader != NULL) {
        HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile);
        free(dataBase->dataHeader);
    }
    if (dataBase->dataFile != NULL) FileRepository_close(dataBase->dataFile);
    free(dataBase);
}