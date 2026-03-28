#include "file_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define CREATE_MODE "w+b"
#define EDIT_MODE "r+b"

#define FILE_STATUS_BYTES_COUNT 1
#define FILE_STATUS_BYTE_OFFSET 0
#define DATA_FILE_BYTE_OFFSET 1

struct DataFile {
    FILE *file;
    bool editMode;
    size_t size;
    long byteOffset;
};

bool FileRepository_isDataFileValid(const struct DataFile *dataFile) {
    if (dataFile == NULL || dataFile->file == NULL) {
        printf("ERROR: Invalid File\n");
        return false;
    }
    return true;
}

bool FileRepository_goToAbsolute(struct DataFile *dataFile, const long absOffset) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (fseek(dataFile->file, absOffset, SEEK_SET) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        return false;
    }
    dataFile->byteOffset = absOffset;
    return true;
}

bool FileRepository_goToDataSection(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (dataFile->byteOffset < DATA_FILE_BYTE_OFFSET) {
        if (!FileRepository_goToAbsolute(dataFile, DATA_FILE_BYTE_OFFSET)) {
            printf("ERROR: Failed to go to data section of file\n");
            return false;
        }
    }
    return true;
}

bool FileRepository_startEditMode(struct DataFile *dataFile) {
    if (dataFile->editMode) return true;
    const long byteOffsetInitial = dataFile->byteOffset;
    if (!FileRepository_goToAbsolute(dataFile, FILE_STATUS_BYTE_OFFSET)) {
        printf("ERROR: Failed go to byte offset of consistent mark file\n");
        return false;
    }
    const bool consistent = false;
    if (fwrite(&consistent, FILE_STATUS_BYTES_COUNT, 1, dataFile->file) != 1) {
        printf("ERROR: Failed to mark file as inconsistent\n");
        return false;
    }
    dataFile->editMode = true;
    return FileRepository_goToAbsolute(dataFile, byteOffsetInitial);
}

bool FileRepository_finishEditMode(struct DataFile *dataFile) {
    if (!dataFile->editMode) return true;
    if (!FileRepository_goToAbsolute(dataFile, FILE_STATUS_BYTE_OFFSET)) {
        printf("ERROR: Failed to go to status byte\n");
        return false;
    }
    const bool consistent = true;
    if (fwrite(&consistent, FILE_STATUS_BYTES_COUNT, 1, dataFile->file) != 1) {
        printf("ERROR: Failed to mark file as consistent\n");
        return false;
    }
    dataFile->editMode = false;
    return FileRepository_goToAbsolute(dataFile, DATA_FILE_BYTE_OFFSET);
}

bool FileRepository_createFile(const String path, struct DataFile *dataFile) {
    dataFile->file = fopen(path, CREATE_MODE);
    dataFile->byteOffset = FILE_STATUS_BYTE_OFFSET;
    dataFile->size = 0;
    return dataFile->file != NULL;
}

bool FileRepository_updateFileSize(struct DataFile *dataFile) {
    if (fseek(dataFile->file, 0, SEEK_END) != 0) return false;
    const long fileSize = ftell(dataFile->file);
    if (fileSize < 0) return false;
    dataFile->size = fileSize;
    return true;
}

struct DataFile *FileRepository_openOrCreate(const String path) {
    struct DataFile *dataFile = malloc(sizeof(struct DataFile));
    if (!dataFile) return NULL;
    dataFile->editMode = false;
    dataFile->file = fopen(path, EDIT_MODE);
    if (!dataFile->file) {
        if (!FileRepository_createFile(path, dataFile)) {
            printf("ERROR: Failed to create file\n");
            free(dataFile);
            return NULL;
        }
        return dataFile;
    }

    if (!FileRepository_updateFileSize(dataFile)) {
        printf("ERROR: Failed to get file size\n");
        fclose(dataFile->file);
        free(dataFile);
        return NULL;
    }
    if (fseek(dataFile->file, FILE_STATUS_BYTE_OFFSET, SEEK_SET) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        free(dataFile);
        return NULL;
    }
    dataFile->byteOffset = FILE_STATUS_BYTE_OFFSET;

    if (dataFile->size >= FILE_STATUS_BYTES_COUNT) {
        if (fseek(dataFile->file, FILE_STATUS_BYTE_OFFSET, SEEK_SET) != 0) {
            printf("ERROR: Failed to reposition cursor\n");
            fclose(dataFile->file);
            free(dataFile);
            return NULL;
        }
        bool consistent;
        if (fread(&consistent, FILE_STATUS_BYTES_COUNT, 1, dataFile->file) != 1) {
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
        FileRepository_goToAbsolute(dataFile, DATA_FILE_BYTE_OFFSET);
    }
    return dataFile;
}

bool FileRepository_read(struct DataFile *dataFile, const size_t elementSize, const size_t count, void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    const size_t bytesCount = elementSize * count;
    if (!FileRepository_goToDataSection(dataFile)) return false;
    if (dataFile->byteOffset + bytesCount > dataFile->size) {
        printf("ERROR: Invalid ByteOffset access\n");
        return false;
    }
    if (fread(buffer, elementSize, count, dataFile->file) != count) {
        printf("ERROR: Could not read %zu bytes\n", bytesCount);
        return false;
    }
    dataFile->byteOffset += (long) bytesCount;
    return true;
}

bool FileRepository_write(struct DataFile *dataFile, const size_t elementSize, const size_t count, const void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (buffer == NULL) {
        printf("ERROR: Invalid Buffer\n");
        return false;
    }
    if (!FileRepository_goToDataSection(dataFile)) return false;

    const size_t bytesCount = elementSize * count;
    const size_t byteOffsetFinal = dataFile->byteOffset + bytesCount;
    if (dataFile->size < byteOffsetFinal) dataFile->size = byteOffsetFinal;
    if (!FileRepository_startEditMode(dataFile)) return false;
    if (fwrite(buffer, elementSize, count, dataFile->file) != count) {
        printf("ERROR: Failed to write data\n");
        return false;
    }
    dataFile->byteOffset += (long) bytesCount;
    return true;
}

size_t FileRepository_fileSize(const struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return 0;
    return dataFile->size < FILE_STATUS_BYTES_COUNT ? 0 : dataFile->size - FILE_STATUS_BYTES_COUNT;
}

bool FileRepository_move(struct DataFile *dataFile, const long movement) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    const long byteOffsetFinal = dataFile->byteOffset + movement;
    if (byteOffsetFinal < DATA_FILE_BYTE_OFFSET || byteOffsetFinal > dataFile->size) {
        printf("ERROR: Invalid ByteOffset access\n");
        return false;
    }
    if (fseek(dataFile->file, movement, SEEK_CUR) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        return false;
    }
    dataFile->byteOffset = byteOffsetFinal;
    return true;
}

bool FileRepository_moveUntil(struct DataFile *dataFile, const long byteOffset) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    const long absByteOffset = byteOffset + DATA_FILE_BYTE_OFFSET;
    const long movement = absByteOffset - dataFile->byteOffset;
    return FileRepository_move(dataFile, movement);
}

bool FileRepository_goTo(struct DataFile *dataFile, const long byteOffset) {
    const long absByteOffset = byteOffset + DATA_FILE_BYTE_OFFSET;
    return FileRepository_goToAbsolute(dataFile, absByteOffset);
}

bool FileRepository_readBool(struct DataFile *dataFile, bool *result) {
    return FileRepository_read(dataFile, UINT8_BYTES_COUNT, 1, result);
}

bool FileRepository_readByte(struct DataFile *dataFile, uint8_t *result) {
    return FileRepository_read(dataFile, UINT8_BYTES_COUNT, 1, result);
}

bool FileRepository_readInt(struct DataFile *dataFile, uint32_t *result) {
    return FileRepository_read(dataFile, UINT32_BYTES_COUNT, 1, result);
}

bool FileRepository_readString(struct DataFile *dataFile, const size_t length, char *result) {
    if (!FileRepository_read(dataFile, UINT8_BYTES_COUNT, length, result)) return false;
    result[length] = '\0';
    return true;
}

bool FileRepository_writeBool(struct DataFile *dataFile, const bool value) {
    const uint8_t byte = value ? 1 : 0;
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, 1, &byte);
}

bool FileRepository_writeByte(struct DataFile *dataFile, const uint8_t value) {
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, 1, &value);
}

bool FileRepository_writeInt(struct DataFile *dataFile, const uint32_t value) {
    return FileRepository_write(dataFile, UINT32_BYTES_COUNT, 1, &value);
}

bool FileRepository_writeString(struct DataFile *dataFile, const size_t length, const char *value) {
    if (!value) {
        printf("ERROR: Invalid String\n");
        return false;
    }
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, length, value);
}

bool FileRepository_flush(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (!FileRepository_finishEditMode(dataFile)) return false;
    fflush(dataFile->file);
    return true;
}

void FileRepository_close(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return;
    if (dataFile->editMode) FileRepository_flush(dataFile);
    fclose(dataFile->file);
    free(dataFile);
}
