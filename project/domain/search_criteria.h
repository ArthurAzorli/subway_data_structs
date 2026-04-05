/**
 * @file search_criteria.h
 * @brief Defines the search criteria structure for filtering subway records.
 */

#ifndef TRABALHO01_SEARCH_CRITERIA_H
#define TRABALHO01_SEARCH_CRITERIA_H

#include "record_fields.h"

/**
 * @struct SearchCriteria
 * @brief Represents a single search criterion for filtering subway records.
 *
 * This structure encapsulates a field identifier and a corresponding value that can be used
 * to search, filter, or compare subway records based on specific attributes.
 */
struct SearchCriteria {
    enum RecordFields field;  /**< The specific field of the record to compare against */
    char *value;             /**< The value to compare the record's field against (as a string) */
};



#endif //TRABALHO01_SEARCH_CRITERIA_H