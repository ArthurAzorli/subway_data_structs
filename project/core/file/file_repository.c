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
    size_t byteOffset;
};

struct DataFile *DataFileFileRepository_openOrCreate(const String path) {
    struct DataFile *dataFile = malloc(sizeof(struct DataFile));
    if (!dataFile) return NULL;
    dataFile->editMode = false;
    dataFile->file = fopen(path, EDIT_MODE);
    if (!dataFile->file) {
        dataFile->file = fopen(path, CREATE_MODE);
        dataFile->byteOffset = 0;
        dataFile->size = 0;
        if (!dataFile->file) {
            printf("ERROR: Failed to create file\n");
            free(dataFile);
            return NULL;
        }
        return dataFile;
    }

    if (fseek(dataFile->file, 0, SEEK_END) != 0) {
        printf("ERROR: Failed to get file size\n");
        free(dataFile);
        return NULL;
    }

    const long fileSize = ftell(dataFile->file);
    if (fileSize < 0) {
        printf("ERROR: Failed to get file size\n");
        free(dataFile);
        return NULL;
    } else if (fileSize < 1) {
        dataFile->byteOffset = 0;
        dataFile->size = 0;
        return dataFile;
    }

    dataFile->size = fileSize;
    dataFile->byteOffset = 0;
    if (fseek(dataFile->file, 0, SEEK_SET) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        free(dataFile);
        return NULL;
    }

    bool consistent;
    if (fread(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file) != UINT8_BYTES_COUNT) {
        printf("ERROR: Failed to read consistent mark file\n");
        fclose(dataFile->file);
        free(dataFile);
        return NULL;
    }
    dataFile->byteOffset = 1;
    if (!consistent) {
        printf("ERROR: inconsistent file\n");
        fclose(dataFile->file);
        free(dataFile);
        return NULL;
    }
    FileRepository_goTo(dataFile, 1);
    return dataFile;
}

bool FileRepository_isDataFileValid(const struct DataFile *dataFile) {
    if (dataFile == NULL || dataFile->file == NULL) {
        printf("ERROR: Invalid File\n");
        return false;
    }
    return true;
}

bool FileRepository_read(struct DataFile *dataFile, const size_t bytesCount, void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (dataFile->byteOffset + bytesCount > dataFile->size) {
        printf("ERROR: Invalid ByteOffset access\n");
        return false;
    }
    if (dataFile->byteOffset == 0 && !FileRepository_goTo(dataFile, 1)) {
        printf("ERROR: Failed to go to data byte offset\n");
        return false;
    }

    const size_t count = fread(buffer, UINT8_BYTES_COUNT, bytesCount, dataFile->file);
    dataFile->byteOffset += count;
    return count == bytesCount;
}

bool FileRepository_write(struct DataFile *dataFile, const size_t bytesCount,
                          const void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;

    if (buffer == NULL) {
        printf("ERROR: Invalid Buffer\n");
        return false;
    }

    const size_t byteOffsetFinal = dataFile->byteOffset + bytesCount;
    dataFile->size = dataFile->size > byteOffsetFinal ? dataFile->size : byteOffsetFinal;

    if (!dataFile->editMode) {
        if (!FileRepository_goTo(dataFile, 0)) {
            printf("ERROR: Failed go to byte offset of consistent mark file\n");
            return false;
        }
        const bool consistent = false;
        if (fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file) != UINT8_BYTES_COUNT) {
            printf("ERROR: Failed to mark file as inconsistent\n");
            return false;
        }
        dataFile->editMode = true;
    }

    if (dataFile->byteOffset == 0 && !FileRepository_goTo(dataFile, 1)) {
        printf("ERROR: Failed to go to data byte offset\n");
        return false;
    }

    const size_t count = fwrite(buffer, UINT8_BYTES_COUNT, bytesCount, dataFile->file);
    dataFile->byteOffset += count;
    return count == bytesCount;
}

size_t FileRepository_fileSize(const struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return 0;
    return dataFile->size;
}

bool FileRepository_goTo(struct DataFile *dataFile, const long byteOffset) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (byteOffset > dataFile->size || byteOffset < 0) {
        printf("ERROR: Invalid ByteOffset access\n");
        return false;
    }
    if (byteOffset == dataFile->byteOffset) return true;
    if (fseek(dataFile->file, byteOffset, SEEK_SET) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        return false;
    }
    dataFile->byteOffset = byteOffset;
    return true;
}

bool FileRepository_readBool(struct DataFile *dataFile, bool *result) {
    uint8_t buffer;
    if (!FileRepository_read(dataFile, UINT8_BYTES_COUNT, &buffer)) return false;
    *result = buffer != 0;
    return true;
}

bool FileRepository_readByte(struct DataFile *dataFile, uint8_t *result) {
    return FileRepository_read(dataFile, UINT8_BYTES_COUNT, result);
}

bool FileRepository_readInt(struct DataFile *dataFile, const Endianness endianness, uint32_t *result) {
    *result = 0;
    uint8_t buffer[UINT32_BYTES_COUNT];
    if (!FileRepository_read(dataFile, UINT32_BYTES_COUNT, buffer)) return false;
    for (size_t i = 0; i < UINT32_BYTES_COUNT; i++) {
        const uint8_t byte = buffer[i];
        const size_t bitOffset = endianness == BIG_ENDIAN ? UINT32_BYTES_COUNT - 1 - i : i;
        *result |= (byte & 0xFF) << (8 * bitOffset);
    }
    return true;
}

bool FileRepository_readString(struct DataFile *dataFile, const size_t length,
                               char *result) {
    if (!FileRepository_read(dataFile, length, result)) return false;
    result[length] = '\0';
    return true;
}

bool FileRepository_writeBool(struct DataFile *dataFile, const bool value) {
    const uint8_t byte = value != 0;
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, &byte);
}

bool FileRepository_writeByte(struct DataFile *dataFile, const uint8_t value) {
    const uint8_t byte = value & 0xFF;
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, &byte);
}

bool FileRepository_writeInt(struct DataFile *dataFile, const Endianness endianness, const uint32_t value) {
    uint8_t buffer[UINT32_BYTES_COUNT];
    for (size_t i = 0; i < UINT32_BYTES_COUNT; i++) {
        const size_t bitOffset = endianness == BIG_ENDIAN ? UINT32_BYTES_COUNT - 1 - i : i;
        const uint8_t byte = value >> (8 * bitOffset) & 0xFF;
        buffer[i] = byte;
    }
    return FileRepository_write(dataFile, UINT32_BYTES_COUNT, buffer);
}

bool FileRepository_writeString(struct DataFile *dataFile, const size_t length,
                                const char *value) {
    if (value == NULL) {
        printf("ERROR: Invalid String\n");
        return false;
    }
    return FileRepository_write(dataFile, length, (const uint8_t *) value);
}

bool FileRepository_flush(struct DataFile *dataFile) {
    if (!dataFile->editMode) return true;
    if (!FileRepository_goTo(dataFile, 0)) return false;
    const bool consistent = true;
    const size_t count = fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file);
    if (count != UINT8_BYTES_COUNT) {
        printf("ERROR: Failed to mark file as consistent\n");
        return false;
    }
    dataFile->editMode = false;
    if (!FileRepository_goTo(dataFile, 1)) return false;
    return true;
}

void FileRepository_close(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return;
    if (dataFile->editMode) FileRepository_flush(dataFile);
    fclose(dataFile->file);
    free(dataFile);
}
