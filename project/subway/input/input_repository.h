/**
 * @file input_repository.h
 * @brief CSV input file parsing and record extraction.
 *
 * This module handles opening and reading records from CSV input files,
 * parsing fields and converting them into SubwayRecord structures.
 */


#ifndef TRABALHO01_INPUT_REPOSITORY_H
#define TRABALHO01_INPUT_REPOSITORY_H

#include "../subway_record_repository.h"
#include "../../services/utils/types.h"

struct InputFile;

struct InputFile *InputRepository_openFile(String path);

struct SubwayRecord *InputRepository_extractRecord(struct InputFile *inputFile);

void InputRepository_closeFile(struct InputFile *inputFile);


#endif //TRABALHO01_INPUT_REPOSITORY_H
