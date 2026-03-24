#include "subway_line_record.h"

#include <stdio.h>
#include <stdlib.h>
#include "../core/utils/types.h"

struct SubwayLineRecord * SubwayLineRecord_init() {
    struct SubwayLineRecord* record = malloc(sizeof(struct SubwayLineRecord));
    if (record == NULL) {
        printf("ERROR: Failed to allocate data record\n");
        return NULL;
    }
    record->rrn = EMPTY;
    record->originStationID = EMPTY;
    record->originLineID = EMPTY;
    record->destinationStationID = EMPTY;
    record->destinationDistant = EMPTY;
    record->interactionLineID = EMPTY;
    record->interactionStationID = EMPTY;
    record->stationNameLength = 0;
    record->stationName = NULL;
    record->lineNameLength = 0;
    record->lineName = NULL;
    return record;
}

void SubwayLineRecord_free(struct SubwayLineRecord *record) {
    if (record == NULL) return;
    if (record->stationName != NULL) free(record->stationName);
    if (record->lineName != NULL) free(record->lineName);
    free(record);
}
