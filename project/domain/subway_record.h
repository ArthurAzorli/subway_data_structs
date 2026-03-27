#ifndef SUBWAY_LINE_RECORD_H
#define SUBWAY_LINE_RECORD_H

#include <stdint.h>

struct SubwayRecord {
    size_t rrn;
    uint32_t originStationID;
    uint32_t originLineID;
    uint32_t destinationStationID;
    uint32_t destinationDistant;
    uint32_t interactionStationID;
    uint32_t interactionLineID;
    uint32_t stationNameLength;
    char *stationName;
    uint32_t lineNameLength;
    char *lineName;
};

struct SubwayRecord *SubwayRecord_init();

void SubwayRecord_free(struct SubwayRecord *record);

#endif //SUBWAY_LINE_RECORD_H
