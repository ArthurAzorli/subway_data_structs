//
// Created by arthu on 25/05/2026.
//

#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H

#include <stdbool.h>
#include <stdint.h>

#include "../utils/types.h"

enum DataType {BOOLEAN, INTEGER, CHAR};

enum FileMode {WRITE_ONLY, READ_ONLY, READ_WRITE};

struct DataFile;

struct DataFile* FileRepository_openOrCreate(const char path[], enum FileMode mode);

size_t FileRepository_fileSize(const struct DataFile* file);

bool FileRepository_move(struct DataFile* file, long movement);

bool FileRepository_goto(struct DataFile* file, long byteOffset);

bool FileRepository_write(struct DataFile* file, enum DataType type, const void *buffer);

bool FileRepository_write(struct DataFile* file, enum DataType type, const void *buffer, size_t count);

bool FileRepository_read(struct DataFile* file, enum DataType type, void *buffer);

bool FileRepository_read(struct DataFile* file, enum DataType type, void *buffer, size_t count);

bool FileRepository_flush(struct DataFile* file);

void FileRepository_close(struct DataFile* file);

#endif //FILE_REPOSITORY_H
