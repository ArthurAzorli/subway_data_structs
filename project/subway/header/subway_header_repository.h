#ifndef SUBWAY_HEADER_REPOSITORY_H
#define SUBWAY_HEADER_REPOSITORY_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @struct DataHeader
 * @brief Metadata header for the database file.
 */
struct DataSubwayHeader {
    uint32_t nextInsert; /**< RRN (Relative Record Number) for the next new record insertion */
    uint32_t lastRemoved; /**< RRN of the most recently removed record (for reuse chain) */
    uint32_t stationsCount; /**< Total count of unique station names in database */
    uint32_t pairStationsCount; /**< Total count of unique station pairs (routes) in database */
};

struct DataSubwayHeader *SubwayHeaderRepository_init(struct DataFile *dataFile);

bool SubwayHeaderRepository_save(const struct DataSubwayHeader *header, struct DataFile *dataFile);

#endif //SUBWAY_HEADER_REPOSITORY_H
