#include "file_repository.h"

#include <stdlib.h>

#include "../bytes/byte_stream.h"

#define CREATE_MODE "w+b"
#define EDIT_MODE "r+b"

#define  UINT8_BYTES_COUNT 1
#define  UINT16_BYTES_COUNT 2
#define  UINT32_BYTES_COUNT 4
#define  UINT64_BYTES_COUNT 8

struct DataFile {
    FILE *file;
    bool editMode;
    size_t size;
};

struct DataFile *DataFileFileRepository_openOrCreate(const String path) {
    struct DataFile *dataFile = malloc(sizeof(struct DataFile));
    if (!dataFile) return NULL;
    dataFile->editMode = false;
    dataFile->file = fopen(path, EDIT_MODE);
    if (!dataFile->file) {
        dataFile->file = fopen(path, CREATE_MODE);
        dataFile->size = 0;
        if (!dataFile->file) {
            printf("ERROR: Failed to create file\n");
            free(dataFile);
            return NULL;
        }
    } else {
        fseek(dataFile->file, 0, SEEK_END);
        dataFile->size = ftell(dataFile->file);
        bool consistent;
        if (!FileRepository_readBool(dataFile, 0, &consistent)) {
            printf("ERROR: Failed to read consistent mark file\n");
            fclose(dataFile->file);
            free(dataFile);
            return NULL;
        }
        if (!consistent) {
            printf("ERROR: inconsistent file\n");
            fclose(dataFile->file);
            free(dataFile);
            return NULL;
        }
    }

    return dataFile;
}

bool FileRepository_isDataFileValid(const struct DataFile *dataFile) {
    if (dataFile == NULL || dataFile->file == NULL) {
        printf("ERROR: Invalid File\n");
        return false;
    }
    return true;
}

bool FileRepository_read(const struct DataFile *dataFile, const long byteOffset, const size_t bytesCount,
                         void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (byteOffset + bytesCount > dataFile->size) {
        printf("ERROR: Invalid ByteOffset access\n");
        return false;
    }

    fseek(dataFile->file, byteOffset, SEEK_SET);
    const size_t count = fread(buffer, UINT8_BYTES_COUNT, bytesCount, dataFile->file);
    return count == bytesCount;
}

bool FileRepository_write(struct DataFile *dataFile, const long byteOffset, const size_t bytesCount,
                          const uint8_t *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;

    if (buffer == NULL) {
        printf("ERROR: Invalid Buffer\n");
        return false;
    }

    dataFile->size = dataFile->size > byteOffset + bytesCount ? dataFile->size : byteOffset + bytesCount;

    if (!dataFile->editMode) {
        fseek(dataFile->file, 0, SEEK_SET);
        const bool consistent = false;
        if (fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file) != UINT8_BYTES_COUNT) {
            printf("ERROR: Failed to mark file as inconsistent\n");
            return false;
        }
        dataFile->editMode = true;
    }

    fseek(dataFile->file, byteOffset, SEEK_SET);
    const size_t count = fwrite(buffer, UINT8_BYTES_COUNT, bytesCount, dataFile->file);
    return count == bytesCount;
}

bool FileRepository_readBool(const struct DataFile *dataFile, const long byteOffset, bool *result) {
    return FileRepository_read(dataFile, byteOffset, UINT8_BYTES_COUNT, result);
}

bool FileRepository_readByte(const struct DataFile *dataFile, const long byteOffset, uint8_t *result) {
    return FileRepository_read(dataFile, byteOffset, UINT8_BYTES_COUNT, result);
}

bool FileRepository_readInt(const struct DataFile *dataFile, const long byteOffset, uint32_t *result) {
    *result = 0;
    uint8_t buffer[UINT32_BYTES_COUNT];
    if (!FileRepository_read(dataFile, byteOffset, UINT32_BYTES_COUNT, buffer)) return false;
    for (size_t i = 0; i < UINT32_BYTES_COUNT; i++) {
        const uint8_t byte = buffer[i];
        *result |= (byte & 0xFF) << (8 * i);
    }
    return true;
}

bool FileRepository_readString(const struct DataFile *dataFile, const long byteOffset, const size_t length,
                               char *result) {
    if (!FileRepository_read(dataFile, byteOffset, length, result)) return false;
    result[length] = '\0';
    return true;
}

bool FileRepository_writeBool(struct DataFile *dataFile, const long byteOffset, const bool value) {
    const uint8_t byte = value & 0xFF;
    return FileRepository_write(dataFile, byteOffset, UINT8_BYTES_COUNT, &byte);
}

bool FileRepository_writeByte(struct DataFile *dataFile, const long byteOffset, const uint8_t value) {
    const uint8_t byte = value & 0xFF;
    return FileRepository_write(dataFile, byteOffset, UINT8_BYTES_COUNT, &byte);
}

bool FileRepository_writeInt(struct DataFile *dataFile, const long byteOffset, const uint32_t value) {
    uint8_t buffer[UINT32_BYTES_COUNT];
    for (size_t i = 0; i < UINT32_BYTES_COUNT; i++) {
        const uint8_t byte = value >> (8 * i) & 0xFF;
        buffer[i] = byte;
    }
    return FileRepository_write(dataFile, byteOffset, UINT32_BYTES_COUNT, buffer);
}

bool FileRepository_writeString(struct DataFile *dataFile, const long byteOffset, const size_t length,
                                const char *value) {
    if (value == NULL) {
        printf("ERROR: Invalid String\n");
        return false;
    }
    return FileRepository_write(dataFile, byteOffset, length, (const uint8_t *) value);
}

bool FileRepository_flush(struct DataFile *dataFile) {
    if (!dataFile->editMode) return true;
    fseek(dataFile->file, 0, SEEK_SET);
    const bool consistent = true;
    const size_t count = fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file);
    if (count != 1) {
        printf("ERROR: Failed to mark file as consistent\n");
        return false;
    }
    dataFile->editMode = false;
    return true;
}

void FileRepository_close(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return;
    if (dataFile->editMode) FileRepository_flush(dataFile);
    fclose(dataFile->file);
    free(dataFile);
}
