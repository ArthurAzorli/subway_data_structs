#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum DataType {BOOLEAN, INTEGER, CHAR};

enum FileMode {WRITE_ONLY, READ_ONLY, READ_WRITE};

struct DataFile;

struct DataFile* FileRepository_openOrCreate(const char path[], enum FileMode mode);

void FileRepository_move(struct DataFile* file, long movement);

void FileRepository_goto(struct DataFile* file, long byteOffset);

bool FileRepository_write(struct DataFile* file, enum DataType type, const void *buffer, size_t count);

bool FileRepository_read(struct DataFile* file, enum DataType type, void *buffer, size_t count);

void FileRepository_close(struct DataFile* file);

#endif //FILE_REPOSITORY_H
