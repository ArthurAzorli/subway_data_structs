//
// Created by arthu on 15/06/2026.
//

#ifndef SEARCH_SERVICE_H
#define SEARCH_SERVICE_H

#include "../indexable/indexable_record_avl.h"
#include "../subway/subway_header_repository.h"
#include "../subway/subway_record_list.h"

#define FIELDS_MAX_COUNT 8

/**
 * @enum SubwayRecordFields
 * @brief Enumeration of all searchable and comparable fields in a subway record.
 *
 * These fields represent the individual components of a SubwayRecord that can be used
 * for searching, filtering, and comparison operations.
 */
enum SubwayRecordFields {
    StationID, /**< Origin station ID field */
    StationName, /**< Origin station name field */
    LineID, /**< Line ID field */
    LineName, /**< Line name field */
    DestinationStationID, /**< Destination station ID field */
    Distant, /**< Distance to destination field */
    InteractionStationID, /**< Interchange station ID field */
    InteractionLineID, /**< Interchange line ID field */
};


struct SubwayFieldValue {
    enum SubwayRecordFields field;
    char value[101];
};

bool SearchService_readSubwayFields(struct SubwayFieldValue *fields, size_t *fieldsCount, bool *hasStationID);

bool SearchService_sequencialSearch(const struct DataSubwayHeader *header, struct DataFile *file,
                                    struct SubwayRecordList *subwayList);

bool SearchService_indexableSearch(const struct DataSubwayHeader *header, struct DataFile *file,
                                       const struct IndexableRecordAVL *avl,
                                       struct SubwayRecordList *subwayList);


#endif //SEARCH_SERVICE_H
