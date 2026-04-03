//
// Created by User on 03/04/2026.
//

#ifndef TRABALHO01_INPUT_REPOSITORY_H
#define TRABALHO01_INPUT_REPOSITORY_H

#include  "../../domain/subway_record.h"
#include "../../core/utils/types.h"

struct InputFile;

struct InputFile *InputRepository_openFile(String path);

struct SubwayRecord *InputRepository_extractRecord(struct InputFile *inputFile);

void InputRepository_closeFile(struct InputFile *inputFile);


#endif //TRABALHO01_INPUT_REPOSITORY_H