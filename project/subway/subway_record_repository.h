
#ifndef RECORD_REPOSITORY_H
#define RECORD_REPOSITORY_H
#include "../services/file/file_repository.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


/**
 * @struct SubwayRecord
 * @brief Represents a subway station and its connection information.
 */
struct SubwayRecord {
    size_t rrn; /**< Relative Record Number - unique identifier in database */
    uint32_t originStationID; /**< ID of the current station */
    uint32_t originLineID; /**< ID of the subway line */
    uint32_t destinationStationID; /**< ID of the next station on this line */
    uint32_t destinationDistant; /**< Distance to the next station */
    uint32_t interactionStationID; /**< ID of an interchange station (if available) */
    uint32_t interactionLineID; /**< Line ID for interchange station (if available) */
    uint32_t stationNameLength; /**< Length of the station name string */
    char *stationName; /**< Station name string */
    uint32_t lineNameLength; /**< Length of the line name string */
    char *lineName; /**< Line name string */
};

struct SubwayRecord *SubwayRecord_init();

void SubwayRecord_free(struct SubwayRecord *record);

struct SubwayRecord *SubwayRecordRepository_readRecord(struct DataFile *dataFile);

bool SubwayRecordRepository_writeRecord(struct DataFile *dataFile, struct SubwayRecord *record);

bool SubwayRecordRepository_removeRecord(struct DataFile *dataFile, uint32_t lastRemoved);

bool SubwayRecordRepository_readNextRemoved(struct DataFile *dataFile, uint32_t *nextRemoved);


#endif //RECORD_REPOSITORY_H
