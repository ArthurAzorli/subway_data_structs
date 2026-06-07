#ifndef SEARCH_CRITERIA_H
#define SEARCH_CRITERIA_H

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

/**
 * @struct SearchCriteria
 * @brief Represents a single search criterion for filtering subway records.
 *
 * This structure encapsulates a field identifier and a corresponding value that can be used
 * to search, filter, or compare subway records based on specific attributes.
 */
struct SearchCriteria {
    enum SubwayRecordFields field; /**< The specific field of the record to compare against */
    char value[101]; /**< The value to compare the record's field against (as a string) */
};


#endif //SEARCH_CRITERIA_H
