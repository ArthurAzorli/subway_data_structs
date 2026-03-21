//
// Created by arthu on 20/03/2026.
//

#include "file_repository.h"

#include "../bytes/byte_stream.h"

struct DataFile {
    FILE *file;
    bool editMode;
    size_t size;
};

bool FileRepository_isDataFileValid(const struct DataFile *dataFile) {
    if (dataFile == NULL || dataFile->file == NULL) {
        printf("ERROR: Invalid File");
        return false;
    }
    return true;
}

bool FileRepository_read(const struct DataFile *dataFile, const size_t byteOffset, const size_t bytesCount,
                         void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (byteOffset + bytesCount > dataFile->size) {
        printf("ERROR: Invalid ByteOffset access");
        return false;
    }

    fseek(dataFile->file, byteOffset, SEEK_SET);
    const int count = fread(buffer, byteOffset, bytesCount, dataFile->file);
    return count != byteOffset;
}

bool FileRepository_write(struct DataFile *dataFile, const size_t byteOffset, const size_t bytesCount,
                          const uint8_t *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;

    if (buffer == NULL) {
        printf("ERROR: Invalid Buffer");
        return false;
    }

    dataFile->size = dataFile->size > byteOffset + bytesCount ? dataFile->size : byteOffset + bytesCount;

    if (!dataFile->editMode) {
        fseek(dataFile->file, 0, SEEK_SET);
        const bool consistent = false;
        const int count = fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file);
        if (count != 1) {
            printf("ERROR: Nao deu certo aqui rapaz");
            return false;
        }
        dataFile->editMode = true;
    }

    fseek(dataFile->file, byteOffset, SEEK_SET);
    const int count = fwrite(buffer, UINT8_BYTES_COUNT, bytesCount, dataFile->file);
    return count != byteOffset;
}

bool FileRepository_readBool(const struct DataFile *dataFile, const size_t byteOffset, bool *result) {
    return FileRepository_read(dataFile, byteOffset, UINT8_BYTES_COUNT, result);
}

bool FileRepository_readByte(const struct DataFile *dataFile, const size_t byteOffset, uint8_t *result) {
    return FileRepository_read(dataFile, byteOffset, UINT8_BYTES_COUNT, result);
}

bool FileRepository_readInt(const struct DataFile *dataFile, const size_t byteOffset, uint32_t *result) {
    //TODO: verificar o endian, dependendo, fds, se não constroi o inteiro a partir de um buffer de bytes
    return FileRepository_read(dataFile, byteOffset, UINT32_BYTES_COUNT, result);
}

bool FileRepository_readString(const struct DataFile *dataFile, const size_t byteOffset, const size_t length,
                               char *result) {
    if (!FileRepository_read(dataFile, byteOffset, length, result)) return false;
    result[length] = '\0';
    return true;
}

bool FileRepository_writeBool(struct DataFile *dataFile, const size_t byteOffset, const bool value) {
    const uint8_t byte = value & 0xFF;
    return FileRepository_write(dataFile, byteOffset, UINT8_BYTES_COUNT, &byte);
}

bool FileRepository_writeByte(struct DataFile *dataFile, const size_t byteOffset, const uint8_t value) {
    const uint8_t byte = value & 0xFF;
    return FileRepository_write(dataFile, byteOffset, UINT8_BYTES_COUNT, &byte);
}

bool FileRepository_writeInt(struct DataFile *dataFile, const size_t byteOffset, const uint32_t value) {
    uint8_t buffer[UINT32_BYTES_COUNT];
    for (size_t i = 0; i < UINT32_BYTES_COUNT; i++) {
        const uint8_t byte = value >> (8 * i) & 0xFF;
        buffer[i] = byte;
    }
    return FileRepository_write(dataFile, byteOffset, UINT32_BYTES_COUNT, buffer);
}

bool FileRepository_writeString(struct DataFile *dataFile, const size_t byteOffset, const size_t length, char *result) {
    return FileRepository_write(dataFile, byteOffset, length, (uint8_t *) result);
}

bool FileRepository_flush(struct DataFile *dataFile) {
    if (!dataFile->editMode) return true;
    fseek(dataFile->file, 0, SEEK_SET);
    const bool consistent = true;
    const int count = fwrite(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file);
    if (count != 1) {
        printf("ERROR: Nao deu certo aqui rapaz");
        return false;
    }
    dataFile->editMode = false;
    return true;
}
