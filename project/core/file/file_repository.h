//
// Created by arthu on 20/03/2026.
//

#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H
#include <stdio.h>
#include "../utils/types.h"

struct DataFile;

struct DataFile *DataFileFileRepository_openOrCreate(String path);

bool FileRepository_readBool(const struct DataFile *dataFile, long byteOffset, bool *result);

bool FileRepository_readByte(const struct DataFile *dataFile, long byteOffset, uint8_t *result);

bool FileRepository_readInt(const struct DataFile *dataFile, long byteOffset, uint32_t *result);

bool FileRepository_readString(const struct DataFile *dataFile, long byteOffset, size_t length, char *result);

bool FileRepository_writeBool(struct DataFile *dataFile, long byteOffset, bool value);

bool FileRepository_writeByte(struct DataFile *dataFile, long byteOffset, uint8_t value);

bool FileRepository_writeInt(struct DataFile *dataFile, long byteOffset, uint32_t value);

bool FileRepository_writeString(struct DataFile *dataFile, long byteOffset, size_t length, const char *value);

bool FileRepository_flush(struct DataFile *dataFile);

void FileRepository_close(struct DataFile *dataFile);


#endif //FILE_REPOSITORY_H
