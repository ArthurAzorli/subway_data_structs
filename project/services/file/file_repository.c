#include "file_repository.h"
#include "../../core/file/file_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define WRITE_MODE "wb"
#define READ_MODE "rb"
#define READ_WRITE_MODE "r+b"

/** @brief Size in bytes for an unsigned 8-bit integer. */
#define  UINT8_BYTES_COUNT 1

/** @brief Size in bytes for an unsigned 32-bit integer. */
#define  UINT32_BYTES_COUNT 4

#define CONSISTENT_MARK '1'
#define INCONSISTENT_MARK '0'

/**
 * @struct DataFile
 * @brief Represents an open binary data file with metadata.
 *
 * This structure maintains the state of an open file including the file pointer,
 * current editing mode, total file size, and virtual byte offset within the data section.
 */
struct DataFile {
    FILE *file; /**< Pointer to the underlying FILE object */
    enum FileMode mode; /**< The mode in which the file was opened (writeOnly, readOnly, readWrite) */
    bool edited; /**< true if file is currently in edit mode, false otherwise */
    size_t size; /**< Total file size in bytes (including 1-byte header) */
    long byteOffset; /**< Current byte offset within the file */
};

/**
 * @brief Updates the cached file size by seeking to the end of file.
 * @param dataFile: The file to get size information from
 * @return true if size was successfully determined, false on error
 */
bool FileRepository_updateFileSize(struct DataFile *dataFile) {
    if (fseek(dataFile->file, 0, SEEK_END) != 0) return false;
    const long fileSize = ftell(dataFile->file);
    if (fileSize < 0) return false;
    dataFile->size = fileSize;
    return true;
}

bool FileRepository_setConsistent(struct DataFile *dataFile, const bool consistent) {
    //se o estado for diferente ou ainda não está escrito, setta a consistencia
    if (dataFile->size > 1 && dataFile->edited == consistent) return true;

    //guarda a posição atual e vai para a posição do valor da consistencia, se precisar
    const long byteOffsetInitial = dataFile->byteOffset;
    if (byteOffsetInitial != 0) fseek(dataFile->file, 0, SEEK_SET);

    //escreve o novo valor de consitencia
    const char consistentMark = consistent ? CONSISTENT_MARK : INCONSISTENT_MARK;
    if (fwrite(&consistentMark, UINT8_BYTES_COUNT, 1, dataFile->file) != 1) return false;
    dataFile->edited = consistent;

    //volta para a posição inicial, se precisar
    if (byteOffsetInitial > 1) fseek(dataFile->file, byteOffsetInitial, SEEK_SET);
    //aumenta o tamanho se antes estava vazio
    if (dataFile->size < 1) dataFile->size = 1;
    return true;
}

struct DataFile *FileRepository_openOrCreate(const char path[], const enum FileMode mode) {
    struct DataFile *dataFile = malloc(sizeof(struct DataFile));
    if (!dataFile) return NULL;

    //init fields
    dataFile->edited = false;
    dataFile->mode = mode;

    //define open/create file strategy
    const char *modeFile = mode == READ_ONLY ? READ_MODE : mode == WRITE_ONLY ? WRITE_MODE : READ_WRITE_MODE;

    //try to open file
    dataFile->file = fopen(path, modeFile);
    if (!dataFile->file) {
        free(dataFile);
        return NULL;
    }

    // Initialize size and byte offset virtualization
    if (!FileRepository_updateFileSize(dataFile)) {
        fclose(dataFile->file);
        free(dataFile);
        return NULL;
    }

    if (dataFile->mode == WRITE_ONLY) {
        //go to data section
        fseek(dataFile->file, 1, SEEK_SET) != 0;
        dataFile->byteOffset = 1;
    } else {
        //back to start
        fseek(dataFile->file, 0, SEEK_SET) != 0;
        dataFile->byteOffset = 0;

        // If it already has the consistency marker, read it
        if (dataFile->size >= 1) {
            uint8_t consistent;
            if (fread(&consistent, UINT8_BYTES_COUNT, 1, dataFile->file) != 1 || consistent != CONSISTENT_MARK) {
                fclose(dataFile->file);
                free(dataFile);
                return NULL;
            }
            dataFile->byteOffset = 1;
        }
    }
    return dataFile;
}

size_t FileRepository_fileSize(const struct DataFile* file) {
    return file == NULL ? 0 : file->size;
}

bool FileRepository_move(struct DataFile *file, const long movement) {
    if (file == NULL || file->file == NULL) return false;
    const long byteOffsetFinal = file->byteOffset + movement;
    if (byteOffsetFinal > file->size || byteOffsetFinal < 1) return false;
    fseek(file->file, byteOffsetFinal, SEEK_SET);
    file->byteOffset = byteOffsetFinal;
    return true;
}

bool FileRepository_goto(struct DataFile *file, const long byteOffset) {
    if (file == NULL || file->file == NULL || byteOffset < 0) return false;
    const long absByteOffset = byteOffset + 1;
    if (absByteOffset == file->byteOffset) return true;
    if (absByteOffset > file->size) return false;
    fseek(file->file, absByteOffset, SEEK_SET);
    file->byteOffset = absByteOffset;
    return true;
}

bool FileRepository_write(struct DataFile *file, const enum DataType type, const void *buffer) {
    return FileRepository_write(file, type, buffer, 1);
}

bool FileRepository_write(struct DataFile *file, const enum DataType type, const void *buffer, const size_t count) {
    if (file == NULL || file->file == NULL || buffer == NULL) return false;
    if (file->mode == READ_ONLY) return false;

    const size_t elementSize = type == INTEGER ? UINT32_BYTES_COUNT : UINT8_BYTES_COUNT;

    // Check read positions to not read outside the file
    const size_t bytesCount = elementSize * count;
    const size_t byteOffsetFinal = file->byteOffset + bytesCount;
    if (file->size < byteOffsetFinal) file->size = byteOffsetFinal;

    // Mark as inconsistent if not already
    if (!FileRepository_setConsistent(file, false)) return false;

    //write buffer
    if (fwrite(buffer, elementSize, count, file->file) != count) return false;
    file->byteOffset += (long) bytesCount;
    return true;
}

bool FileRepository_read(struct DataFile *file, const enum DataType type, void *buffer) {
    return FileRepository_read(file, type, buffer, 1);
}

bool FileRepository_read(struct DataFile *file, const enum DataType type, void *buffer, const size_t count) {
    if (file == NULL || file->file == NULL || buffer == NULL) return false;
    if (file->mode == WRITE_ONLY) return false;
    // If not in the data section go to it
    if (file->byteOffset == 0) {
        fseek(file->file, 1, SEEK_SET);
        file->byteOffset = 1;
    }

    const size_t elementSize = type == INTEGER ? UINT32_BYTES_COUNT : UINT8_BYTES_COUNT;

    // Check read positions to not read outside the file
    const size_t bytesCount = elementSize * count;
    const size_t byteOffsetFinal = file->byteOffset + bytesCount;
    if (byteOffsetFinal > file->size) return false;

    //read buffer
    if (fread(buffer, elementSize, count, file->file) != count) return false;
    file->byteOffset += (long) bytesCount;
    return true;
}

bool FileRepository_flush(struct DataFile *file) {
    if (file == NULL || file->file == NULL) return false;
    if (file->mode == READ_ONLY) return true;
    if (!FileRepository_setConsistent(file, true)) return false;
    fflush(file->file);
    return true;
}

void FileRepository_close(struct DataFile *file) {
    if (file == NULL || file->file == NULL) return;
    if (file->edited) FileRepository_flush(file);
    fclose(file->file);
    free(file);
}
