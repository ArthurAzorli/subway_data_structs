#include "data_base_repository.h"

#include <stdlib.h>
#include <string.h>

#include "header_repository.h"
#include "record_repository.h"
#include "../../core/file/file_repository.h"
#include "../../core/utils/errors.h"

/**
 * @brief Validates that a DataBase structure is properly initialized.
 * @param dataBase: The database to validate
 * @return true if database is valid, false otherwise
 */
bool DataBaseRepository_isDataBaseValid(const struct DataBase *dataBase) {
    if (dataBase == NULL || dataBase->dataFile == NULL || dataBase->dataHeader == NULL) {
        throwError("Invalid data base");
        return false;
    }
    return true;
}

/**
 * @brief Validates that an RRN is within valid bounds for the database.
 * @param dataBase: The database to check against
 * @param rrn: Relative Record Number to validate
 * @return true if RRN is valid, false otherwise
 */
bool DataBaseRepository_isRRNValid(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    return dataBase->dataHeader->nextInsert > rrn;
}

/**
 * @brief Updates header statistics when a record is modified.
 *
 * Compares the old and new record versions to determine if station name
 * or station pair changed. If changed, updates the header counts for unique
 * stations and unique pairs accordingly.
 *
 * @param dataBase: The database whose header to update
 * @param oldRecord: The original record values
 * @param newRecord: The new record values
 * @return true if counts were changed and should be saved, false if no changes or error
 */
bool DataBaseRepository_updateHeaderCounts(
    const struct DataBase *dataBase,
    const struct SubwayRecord *oldRecord,
    const struct SubwayRecord *newRecord) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;

    // Check if the station name or next station pair was modified
    const bool nameChanged = strcmp(oldRecord->stationName, newRecord->stationName) != 0;
    const bool pairChanged = oldRecord->originStationID != newRecord->originStationID ||
                             oldRecord->destinationStationID != newRecord->destinationStationID;

    if (!nameChanged && !pairChanged) return true;

    // If it was modified, iterate through records to see the quantity of stations with new and old names and the new and old station pair

    size_t oldStationCount = 0;
    size_t newStationCount = 0;
    size_t oldPairCount = 0;
    size_t newPairCount = 0;

    for (size_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
        struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
        if (record == NULL) continue;


        if (record->stationNameLength == oldRecord->stationNameLength &&
            memcmp(record->stationName, oldRecord->stationName, record->stationNameLength) == 0) {
            oldStationCount++;
        }
        if (record->stationNameLength == newRecord->stationNameLength &&
            memcmp(record->stationName, newRecord->stationName, record->stationNameLength) == 0) {
            newStationCount++;
        }


        if (record->originStationID == oldRecord->originStationID &&
            record->destinationStationID == oldRecord->destinationStationID) {
            oldPairCount++;
        }
        if (record->originStationID == newRecord->originStationID &&
            record->destinationStationID == newRecord->destinationStationID) {
            newPairCount++;
        }

        SubwayRecord_free(record);
    }

    // Check if updating the name affects the number of stations

    bool changeCounts = false;
    if (nameChanged) {
        if (oldStationCount == 1) {
            dataBase->dataHeader->stationsCount--;
            changeCounts = true;
        }
        if (newStationCount == 0) {
            dataBase->dataHeader->stationsCount++;
            changeCounts = true;
        }
    }

    // Check if updating the station pair affects the number of station pairs

    if (pairChanged) {
        if (oldPairCount == 1) {
            dataBase->dataHeader->pairStationsCount--;
            changeCounts = true;
        }
        if (newPairCount == 0) {
            dataBase->dataHeader->pairStationsCount++;
            changeCounts = true;
        }
    }

    return changeCounts;
}

/**
 * @brief Counts how many records contain a specific station name.
 * @param dataBase: The database to search in
 * @param stationNameLength: Length of the station name to match
 * @param stationName: The station name to count occurrences of
 * @return Number of records with the matching station name
 */
size_t DataBaseRepository_countStation(const struct DataBase *dataBase, const size_t stationNameLength,
                                       const String stationName) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return 0;
    if (stationName == NULL) {
        throwError("Invalid station name");
        return 0;
    }

    size_t count = 0;
    for (size_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
        struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
        if (record == NULL || record->stationName == NULL) continue;
        if (record->stationNameLength == stationNameLength && memcmp(record->stationName, stationName,
                                                                     stationNameLength) == 0) {
            count++;
        }
        SubwayRecord_free(record);
    }
    return count;
}

/**
 * @brief Counts how many records have the same origin-destination station pair.
 * @param dataBase: The database to search in
 * @param record: The record whose pair to count (must be valid)
 * @return Number of records with matching station pair
 */
size_t DatabaseRepository_countStationsPairs(const struct DataBase *dataBase, const struct SubwayRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase) || record == NULL) return 0;
    if (record->originStationID == EMPTY || record->destinationStationID == EMPTY) return 0;

    size_t count = 0;
    for (size_t rrn = 0; rrn < dataBase->dataHeader->nextInsert; rrn++) {
        struct SubwayRecord *other = RecordRepository_readRecord(dataBase->dataFile, rrn);
        if (other == NULL) continue;

        if (record->originStationID == other->originStationID &&
            record->destinationStationID == other->destinationStationID)
            count++;
        SubwayRecord_free(other);
    }
    return count;
}

/**
 * @brief Initializes a database from file or creates a new one.
 *
 * Opens or creates a database file at the given path, initializes the header,
 * and returns a ready-to-use DataBase structure. If the file doesn't exist,
 * a new database is created with empty header.
 *
 * @param path: File path for the database
 * @return Pointer to allocated DataBase on success, NULL on failure
 */
struct DataBase *DataBaseRepository_init(String path) {
    // Open the file
    struct DataFile *dataFile = FileRepository_openOrCreate(path);
    if (dataFile == NULL) {
        throwError("Failed to open data base file");
        return NULL;
    }

    // Initialize the header
    struct DataHeader *dataHeader = HeaderRepository_init(dataFile);
    if (dataHeader == NULL) {
        throwError("Failed to initialize data header");
        FileRepository_close(dataFile);
        return NULL;
    }

    struct DataBase *database = malloc(sizeof(struct DataBase));
    if (database == NULL) {
        throwError("Failed to allocate memory for data base");
        FileRepository_close(dataFile);
        free(dataHeader);
        return NULL;
    }

    database->dataHeader = dataHeader;
    database->dataFile = dataFile;

    return database;
}

/**
 * @brief Creates a new record in the database.
 *
 * Inserts a new subway record into the database, assigning it an RRN (Relative Record Number).
 * If there are deleted records available for reuse, their RRNs will be reused instead of
 * incrementing nextInsert. Updates the header with new station and station pair counts.
 *
 * @param dataBase: The database where the record will be created
 * @param record: The subway record to create (will be assigned an RRN)
 * @return true if record was successfully created, false otherwise
 */
bool DataBaseRepository_createRecord(const struct DataBase *dataBase, struct SubwayRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;

    bool reuse = false;
    uint32_t rrn = dataBase->dataHeader->nextInsert;
    const uint32_t lastRemoved = dataBase->dataHeader->lastRemoved;
    // If there is any removed record, reuse its RRN to create the new record
    if (lastRemoved != EMPTY) {
        rrn = lastRemoved;
        if (!RecordRepository_readLastRemoved(dataBase->dataFile, lastRemoved, &dataBase->dataHeader->lastRemoved)) {
            throwError("Failed to read last removed record");
            return false;
        }
        reuse = true;
    }

    // Save the record
    record->rrn = rrn;
    if (!RecordRepository_writeRecord(dataBase->dataFile, record)) {
        throwError("Failed to write record data file");
        return false;
    }

    // Check header modifications
    if (!reuse) dataBase->dataHeader->nextInsert++;
    if (DataBaseRepository_countStation(dataBase, record->stationNameLength, record->stationName) == 1)
        dataBase->
                dataHeader->stationsCount++;
    if (record->destinationStationID != EMPTY && DatabaseRepository_countStationsPairs(dataBase, record) == 1)
        dataBase
                ->dataHeader->pairStationsCount++;

    // Save the header
    if (!HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile)) {
        throwError("Failed to write to data header file");
        return false;
    }

    return FileRepository_flush(dataBase->dataFile);
}

/**
 * @brief Reads a record from the database by its RRN.
 *
 * Retrieves a subway record from the database using its Relative Record Number (RRN).
 * Validates that the RRN is within bounds before reading. The returned record must be
 * freed by the caller using SubwayRecord_free.
 *
 * @param dataBase: The database to read from
 * @param rrn: Relative Record Number of the record to read
 * @return Pointer to the allocated SubwayRecord on success, NULL on failure or invalid RRN
 */
struct SubwayRecord *DataBaseRepository_readRecord(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return NULL;
    if (!DataBaseRepository_isRRNValid(dataBase, rrn)) {
        throwError("RRN is out of bounds");
        return NULL;
    }

    // Read the record by RRN
    struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
    if (record == NULL) {
        throwError("Failed to read record data file");
        return NULL;
    }

    return record;
}

/**
 * @brief Updates an existing record in the database.
 *
 * Modifies a subway record identified by its RRN. Compares the old and new record states
 * to determine if header counts (stations and station pairs) need updating. Only writes
 * changes to the file if the record actually differs from its current state. Updates the
 * header if station names or station pairs are modified.
 *
 * @param dataBase: The database containing the record to update
 * @param record: The updated subway record with the RRN to identify which record to update
 * @return true if record was successfully updated or was already up-to-date, false otherwise
 */
bool DataBaseRepository_updateRecord(const struct DataBase *dataBase, struct SubwayRecord *record) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRRNValid(dataBase, record->rrn)) return false;

    // Get the current state of the record
    struct SubwayRecord *oldRecord = RecordRepository_readRecord(dataBase->dataFile, record->rrn);
    if (oldRecord == NULL) {
        throwError("Failed to read record data file");
        return false;
    }

    // If the state of new and old are equal, no need to update physically
    if (SubwayRecord_isEquals(oldRecord, record)) {
        SubwayRecord_free(oldRecord);
        return true;
    }

    // Check modifications in header, if any save them
    if (DataBaseRepository_updateHeaderCounts(dataBase, oldRecord, record)) {
        if (!HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile)) {
            throwError("Failed to save header data file");
            SubwayRecord_free(oldRecord);
            return false;
        }
    }

    // Save the record modifications
    if (!RecordRepository_writeRecord(dataBase->dataFile, record)) {
        throwError("Failed to write record data file");
        SubwayRecord_free(oldRecord);
        return false;
    }

    SubwayRecord_free(oldRecord);
    return FileRepository_flush(dataBase->dataFile);
}

/**
 * @brief Marks a record as deleted in the database.
 *
 * Removes a record by marking it as deleted and adding it to the removal chain.
 * The RRN slot becomes available for reuse. Updates header statistics.
 *
 * @param dataBase: The database containing the record
 * @param rrn: Relative Record Number of the record to delete
 * @return true if record was successfully deleted, false otherwise
 */
bool DataBaseRepository_deleteRecord(const struct DataBase *dataBase, const size_t rrn) {
    if (!DataBaseRepository_isDataBaseValid(dataBase)) return false;
    if (!DataBaseRepository_isRRNValid(dataBase, rrn)) {
        throwError("RRN is out of bounds");
        return false;
    }

    // Get the current state of the record
    struct SubwayRecord *record = RecordRepository_readRecord(dataBase->dataFile, rrn);
    if (record == NULL) {
        throwError("Record already removed or invalid");
        return false;
    }

    // Remove the record
    if (!RecordRepository_removeRecord(dataBase->dataFile, rrn, &dataBase->dataHeader->lastRemoved)) {
        throwError("Failed to remove record data file");
        return false;
    }

    // Check changes in header and save them
    if (DataBaseRepository_countStation(dataBase, record->stationNameLength, record->stationName) == 0) dataBase->dataHeader->stationsCount--;
    if (DatabaseRepository_countStationsPairs(dataBase, record) == 0) dataBase->dataHeader->pairStationsCount--;
    if (!HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile)) {
        throwError("Failed to write to data header file");
        return false;
    }
    SubwayRecord_free(record);
    return FileRepository_flush(dataBase->dataFile);
}

/**
 * @brief Closes the database and frees all resources.
 *
 * Saves the header, closes both file and header repositories, and deallocates memory.
 *
 * @param dataBase: The database to close (can be NULL)
 */
void DataBaseRepository_close(struct DataBase *dataBase) {
    if (dataBase == NULL) return;
    if (dataBase->dataHeader != NULL) {
        HeaderRepository_save(dataBase->dataHeader, dataBase->dataFile);
        free(dataBase->dataHeader);
    }
    if (dataBase->dataFile != NULL) FileRepository_close(dataBase->dataFile);
    free(dataBase);
}
