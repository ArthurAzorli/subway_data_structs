//
// Created by arthu on 20/03/2026.
//

#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H
#include <stdio.h>
#include "../utils/types.h"

struct DataFile;

typedef enum {
    LITTLE_ENDIAN,
    BIG_ENDIAN,
} Endianness;

struct DataFile *DataFileFileRepository_openOrCreate(String path);

size_t FileRepository_fileSize(const struct DataFile *dataFile);

bool FileRepository_goTo(struct DataFile *dataFile, long byteOffset);

bool FileRepository_readBool(struct DataFile *dataFile, bool *result);

bool FileRepository_readByte(struct DataFile *dataFile, uint8_t *result);

bool FileRepository_readInt(struct DataFile *dataFile, Endianness endianness,  uint32_t *result);

bool FileRepository_readString(struct DataFile *dataFile, size_t length, char *result);

bool FileRepository_writeBool(struct DataFile *dataFile, bool value);

bool FileRepository_writeByte(struct DataFile *dataFile, uint8_t value);

bool FileRepository_writeInt(struct DataFile *dataFile, Endianness endianness, uint32_t value);

bool FileRepository_writeString(struct DataFile *dataFile, size_t length, String value);

bool FileRepository_flush(struct DataFile *dataFile);

void FileRepository_close(struct DataFile *dataFile);


#endif //FILE_REPOSITORY_H
