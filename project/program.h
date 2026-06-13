/**
 * @file program.h
 * @brief Main program operations and user interface functions.
 *
 * This module provides the primary operations available to the user including
 * file import, record display, searching, and direct RRN access.
 */

#ifndef TRABALHO01_PROGRAM_H
#define TRABALHO01_PROGRAM_H
#include <stdbool.h>

bool Program_initSubwayFile();

bool Program_showRecords();

bool Program_searchRecord();

bool Program_getRecordByRRN();

bool Program_initIndexableFile();

bool Program_searchRecordByIndexable();



#endif //TRABALHO01_PROGRAM_H
