/**
 * @file subway_record.h
 * @brief Data structure and operations for subway station records.
 */

#ifndef SUBWAY_LINE_RECORD_H
#define SUBWAY_LINE_RECORD_H

#include <stdint.h>
#include <stddef.h>
#include "../core/utils/types.h"

/**
 * @struct SubwayRecord
 * @brief Represents a subway station and its connection information.
 */
struct SubwayRecord {
    size_t rrn;                          /**< Relative Record Number - unique identifier in database */
    uint32_t originStationID;            /**< ID of the current station */
    uint32_t originLineID;               /**< ID of the subway line */
    uint32_t destinationStationID;       /**< ID of the next station on this line */
    uint32_t destinationDistant;         /**< Distance to the next station */
    uint32_t interactionStationID;       /**< ID of an interchange station (if available) */
    uint32_t interactionLineID;          /**< Line ID for interchange station (if available) */
    uint32_t stationNameLength;          /**< Length of the station name string */
    char *stationName;                   /**< Station name string */
    uint32_t lineNameLength;             /**< Length of the line name string */
    char *lineName;                      /**< Line name string */
};

struct SubwayRecord *SubwayRecord_init();

bool SubwayRecord_isEquals(const struct SubwayRecord *thisRecord, const struct SubwayRecord *otherRecord);

void SubwayRecord_free(struct SubwayRecord *record);

#endif //SUBWAY_LINE_RECORD_H
