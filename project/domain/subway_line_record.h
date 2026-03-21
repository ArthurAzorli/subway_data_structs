#ifndef SUBWAY_LINE_RECORD_H
#define SUBWAY_LINE_RECORD_H

#include <stdint.h>

struct  SubwayLineRecord {
    size_t* rrn;
    uint32_t *originStationID;
    uint32_t *originLineID;
    uint32_t *destinationStationID;
    uint32_t *destinationLineID;
    uint32_t *interactionStationID;
    uint32_t *interactionLineID;
    size_t stationNameLength;
    char *stationName;
    size_t lineNameLength;
    char *lineName;
};

#endif //SUBWAY_LINE_RECORD_H
