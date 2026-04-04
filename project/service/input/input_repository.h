/**
 * @file input_repository.h
 * @brief CSV input file parsing and record extraction.
 *
 * This module handles opening and reading records from CSV input files,
 * parsing fields and converting them into SubwayRecord structures.
 */

//
// Created by User on 03/04/2026.
//

#ifndef TRABALHO01_INPUT_REPOSITORY_H
#define TRABALHO01_INPUT_REPOSITORY_H

#include  "../../domain/subway_record.h"
#include "../../core/utils/types.h"

/** @brief Opaque structure representing an open input file. */
struct InputFile;

/**
 * @brief Opens a CSV file for reading records.
 *
 * Opens a text file containing comma-separated subway station data in read mode.
 * The first line (header) is automatically skipped during first extraction.
 *
 * @param path: Path to the CSV file
 * @return Pointer to InputFile structure on success, NULL on failure
 *
 * @note The returned pointer must be freed using InputRepository_closeFile()
 */
struct InputFile *InputRepository_openFile(String path);

/**
 * @brief Extracts the next record from the input file.
 *
 * Reads the next line from the CSV file, parses all comma-separated fields,
 * converts them to appropriate types, and returns a SubwayRecord structure.
 * The first call automatically skips the header line.
 *
 * @param inputFile: Open input file to read from
 * @return Pointer to allocated SubwayRecord on success, NULL when EOF or on error
 *
 * @note The returned pointer must be freed using SubwayRecord_free()
 * @note Returns NULL at EOF
 */
struct SubwayRecord *InputRepository_extractRecord(struct InputFile *inputFile);

/**
 * @brief Closes the input file and frees resources.
 *
 * @param inputFile: The input file to close (can be NULL)
 */
void InputRepository_closeFile(struct InputFile *inputFile);


#endif //TRABALHO01_INPUT_REPOSITORY_H
