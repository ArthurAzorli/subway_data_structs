//
// Created by arthu on 20/03/2026.
//

#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H
#include <stdio.h>
#include "../utils/types.h"

struct DataFile;

struct DataFile *DataFileFileRepository_openOrCreate(String path);

bool FileRepository_readBool(const struct DataFile *dataFile, const size_t byteOffset, bool *result);

bool FileRepository_readByte(const struct DataFile *dataFile, const size_t byteOffset, uint8_t *result);

bool FileRepository_readInt(const struct DataFile *dataFile, const size_t byteOffset, uint32_t *result);

bool FileRepository_readString(const struct DataFile *dataFile, const size_t byteOffset, const size_t length, char *result);

bool FileRepository_writeBool(struct DataFile *dataFile, const size_t byteOffset, const bool value);

bool FileRepository_writeByte(struct DataFile *dataFile, const size_t byteOffset, const uint8_t value);

bool FileRepository_writeInt(struct DataFile *dataFile, const size_t byteOffset, const uint32_t value);

bool FileRepository_writeString(struct DataFile *dataFile, const size_t byteOffset, const size_t length, char *result);

bool FileRepository_flush(struct DataFile *dataFile);

void FileRepository_close(struct DataFile *dataFile);


#endif //FILE_REPOSITORY_H
