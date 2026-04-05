/**
 * @file record_fields.h
 * @brief Defines the enumeration of subway record field identifiers.
 */

#ifndef TRABALHO01_RECORD_FIELDS_H
#define TRABALHO01_RECORD_FIELDS_H

/**
 * @enum RecordFields
 * @brief Enumeration of all searchable and comparable fields in a subway record.
 *
 * These fields represent the individual components of a SubwayRecord that can be used
 * for searching, filtering, and comparison operations.
 */
enum RecordFields {
    StationID,              /**< Origin station ID field */
    StationName,            /**< Origin station name field */
    LineID,                 /**< Line ID field */
    LineName,               /**< Line name field */
    DestinationStationID,   /**< Destination station ID field */
    Distant,                /**< Distance to destination field */
    InteractionStationID,   /**< Interchange station ID field */
    InteractionLineID,      /**< Interchange line ID field */
};

#endif //TRABALHO01_RECORD_FIELDS_H