#include "file_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define CREATE_MODE "w+b"
#define EDIT_MODE "r+b"

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

bool FileRepository_goToAbsolute(struct DataFile *dataFile, const long byteOffset) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (byteOffset == dataFile->byteOffset) return true;
    if (fseek(dataFile->file, byteOffset, SEEK_SET) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        return false;
    }
    dataFile->byteOffset = byteOffset;
    return true;
}

struct DataFile *FileRepository_openOrCreate(const String path) {
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
    FileRepository_goToAbsolute(dataFile, 1);
    return dataFile;
}

bool FileRepository_read(struct DataFile *dataFile,  const size_t elementSize, const size_t count, void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    const size_t bytesCount = elementSize * count;
    if (dataFile->byteOffset + bytesCount > dataFile->size) {
        printf("ERROR: Invalid ByteOffset access\n");
        return false;
    }
    if (dataFile->byteOffset == 0 && !FileRepository_goToAbsolute(dataFile, 1)) {
        printf("ERROR: Failed to go to data byte offset\n");
        return false;
    }
    const size_t readCount = fread(buffer, elementSize, count, dataFile->file);
    dataFile->byteOffset += (long) bytesCount;
    return readCount == count;
}

bool FileRepository_write(struct DataFile *dataFile, const size_t elementSize, const size_t count, const void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;

    if (buffer == NULL) {
        printf("ERROR: Invalid Buffer\n");
        return false;
    }

    const size_t bytesCount = elementSize * count;
    const size_t byteOffsetFinal = dataFile->byteOffset + bytesCount;
    dataFile->size = dataFile->size > byteOffsetFinal ? dataFile->size : byteOffsetFinal;

    if (!dataFile->editMode) {
        if (!FileRepository_goToAbsolute(dataFile, 0)) {
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

    if (dataFile->byteOffset == 0 && !FileRepository_goToAbsolute(dataFile, 1)) {
        printf("ERROR: Failed to go to data byte offset\n");
        return false;
    }

    const size_t writeCount = fwrite(buffer, elementSize, count, dataFile->file);
    dataFile->byteOffset += (long) bytesCount;
    return writeCount == count;
}

size_t FileRepository_fileSize(const struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return 0;
    return dataFile->size == 0 ? 0 : dataFile->size - 1;
}

bool FileRepository_move(struct DataFile *dataFile, const long movement) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (movement == 0) return true;
    const long byteOffsetFinal = dataFile->byteOffset + movement;
    if (byteOffsetFinal > dataFile->size || byteOffsetFinal < 1) {
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
    const long movement = (byteOffset + 1) - dataFile->byteOffset;
    return FileRepository_move(dataFile, movement);
}

bool FileRepository_goTo(struct DataFile *dataFile, const long byteOffset) {
    return FileRepository_goToAbsolute(dataFile, byteOffset + 1);
}

bool FileRepository_readBool(struct DataFile *dataFile, bool *result) {
    printf("ESTOU LENDO UM BOOL NA POSICAO: %d %d\n", dataFile->byteOffset, ftell(dataFile->file));
    if (!FileRepository_read(dataFile, UINT8_BYTES_COUNT, 1, result)) return false;
    printf("RESULTADO: %d\n", *result);
    return true;
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
    const uint8_t byte = value != 0;
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, 1, &byte);
}

bool FileRepository_writeByte(struct DataFile *dataFile, const uint8_t value) {
    const uint8_t byte = value & 0xFF;
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, 1, &byte);
}

bool FileRepository_writeInt(struct DataFile *dataFile, const uint32_t value) {
    return FileRepository_write(dataFile, UINT32_BYTES_COUNT, 1, &value);
}

bool FileRepository_writeString(struct DataFile *dataFile, const size_t length,
                                const char *value) {
    if (value == NULL) {
        printf("ERROR: Invalid String\n");
        return false;
    }
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, length, value);
}

bool FileRepository_flush(struct DataFile *dataFile) {
    if (!dataFile->editMode) return true;
    if (!FileRepository_goToAbsolute(dataFile, 0)) return false;
    const bool consistent = true;
    const size_t count = fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file);
    if (count != UINT8_BYTES_COUNT) {
        printf("ERROR: Failed to mark file as consistent\n");
        return false;
    }
    dataFile->editMode = false;
    if (!FileRepository_goToAbsolute(dataFile, 1)) return false;
    return true;
}

void FileRepository_close(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return;
    if (dataFile->editMode) FileRepository_flush(dataFile);
    fclose(dataFile->file);
    free(dataFile);
}
