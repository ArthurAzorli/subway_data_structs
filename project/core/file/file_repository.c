#include "file_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define CREATE_MODE "w+b"
#define EDIT_MODE "r+b"

#define FILE_STATUS_BYTES_COUNT 1
#define FILE_STATUS_BYTE_OFFSET 0
#define DATA_FILE_BYTE_OFFSET 1

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
    FILE *file;           /**< Pointer to the underlying FILE object */
    bool editMode;        /**< true if file is currently in edit mode, false otherwise */
    size_t size;          /**< Total file size in bytes (including 1-byte header) */
    long byteOffset;      /**< Current byte offset within the file */
};

/**
 * @brief Checks if the given data file is valid by verifying that the file pointer is not NULL.
 * @param dataFile: current open dataFile
 * @return boolean indicating whether the data file is valid or not
 */
bool FileRepository_isDataFileValid(const struct DataFile *dataFile) {
    if (dataFile == NULL || dataFile->file == NULL) {
        printf("ERROR: Invalid File\n");
        return false;
    }
    return true;
}

/**
 * @brief Moves the file cursor to an absolute position in the file.
 *
 * Sets the byte offset to an absolute position from the start of the file.
 * Both the internal byteOffset and the actual file pointer are updated.
 *
 * @param dataFile: The file to reposition
 * @param absOffset: Absolute byte offset from start of file
 * @return true if repositioning was successful, false otherwise
 */
bool FileRepository_goToAbsolute(struct DataFile *dataFile, const long absOffset) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (fseek(dataFile->file, absOffset, SEEK_SET) != 0) {
        printf("ERROR: Failed to reposition cursor\n");
        return false;
    }
    dataFile->byteOffset = absOffset;
    return true;
}

/**
 * @brief Ensures the file cursor is positioned at or after the start of the data section.
 *
 * If the current byte offset is before the data section start (after the 1-byte header),
 * this function moves the cursor to the data section start.
 *
 * @param dataFile: The file to check and potentially reposition
 * @return true if cursor is now at or after data section start, false on error
 */
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

/**
 * @brief Initiates edit mode by marking the file as inconsistent.
 *
 * When starting a write operation, marks the file with an inconsistency flag
 * to indicate that modifications are in progress. This allows recovery from
 * incomplete writes if the program crashes.
 *
 * @param dataFile: The file to enter edit mode
 * @return true if edit mode was successfully initiated, false otherwise
 */
bool FileRepository_startEditMode(struct DataFile *dataFile) {
    if (dataFile->editMode) return true;
    const long byteOffsetInitial = dataFile->byteOffset;
    if (!FileRepository_goToAbsolute(dataFile, FILE_STATUS_BYTE_OFFSET)) {
        printf("ERROR: Failed go to byte offset of consistent mark file\n");
        return false;
    }
    const bool consistent = INCONSISTENT_MARK;
    if (fwrite(&consistent, FILE_STATUS_BYTES_COUNT, 1, dataFile->file) != 1) {
        printf("ERROR: Failed to mark file as inconsistent\n");
        return false;
    }
    dataFile->editMode = true;
    return FileRepository_goToAbsolute(dataFile, byteOffsetInitial);
}

/**
 * @brief Completes edit mode by marking the file as consistent.
 *
 * After all write operations are complete, marks the file with a consistency flag
 * to indicate that modifications have been successfully completed.
 *
 * @param dataFile: The file to exit edit mode
 * @return true if edit mode was successfully completed, false otherwise
 */
bool FileRepository_finishEditMode(struct DataFile *dataFile) {
    if (!dataFile->editMode) return true;
    if (!FileRepository_goToAbsolute(dataFile, FILE_STATUS_BYTE_OFFSET)) {
        printf("ERROR: Failed to go to status byte\n");
        return false;
    }
    const bool consistent = CONSISTENT_MARK;
    if (fwrite(&consistent, FILE_STATUS_BYTES_COUNT, 1, dataFile->file) != 1) {
        printf("ERROR: Failed to mark file as consistent\n");
        return false;
    }
    dataFile->editMode = false;
    return FileRepository_goToAbsolute(dataFile, DATA_FILE_BYTE_OFFSET);
}

/**
 * @brief Creates a new file at the specified path in binary write mode.
 *
 * Initializes a new file structure with write+read mode and sets the initial
 * byte offset to point to the status byte.
 *
 * @param path: The file path to create
 * @param dataFile: Pointer to DataFile structure to initialize
 * @return true if file was successfully created, false otherwise
 */
bool FileRepository_createFile(const String path, struct DataFile *dataFile) {
    dataFile->file = fopen(path, CREATE_MODE);
    dataFile->byteOffset = FILE_STATUS_BYTE_OFFSET;
    dataFile->size = 0;
    return dataFile->file != NULL;
}

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

/**
 * @brief Opens an existing file or creates a new one if it doesn't exist.
 *
 * Attempts to open the file at the given path. If the file doesn't exist,
 * a new file is created. Verifies file consistency using a 1-byte status marker.
 *
 * @param path: The file path to open or create
 * @return Pointer to allocated DataFile structure on success, NULL on failure
 */
struct DataFile *FileRepository_openOrCreate(const String path) {
    struct DataFile *dataFile = malloc(sizeof(struct DataFile));
    if (!dataFile) return NULL;
    dataFile->editMode = false;
    // Try to open the file
    dataFile->file = fopen(path, EDIT_MODE);
    if (!dataFile->file) {
        // If the file does not exist, create it
        if (!FileRepository_createFile(path, dataFile)) {
            printf("ERROR: Failed to create file\n");
            free(dataFile);
            return NULL;
        }
        return dataFile;
    }

    // Initialize size and byte offset virtualizations
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

    // If it already has the consistency marker, read it
    if (dataFile->size >= FILE_STATUS_BYTES_COUNT) {
        if (fseek(dataFile->file, FILE_STATUS_BYTE_OFFSET, SEEK_SET) != 0) {
            printf("ERROR: Failed to reposition cursor\n");
            fclose(dataFile->file);
            free(dataFile);
            return NULL;
        }
        uint8_t consistent;
        if (fread(&consistent, FILE_STATUS_BYTES_COUNT, 1, dataFile->file) != 1) {
            printf("ERROR: Failed to read consistent mark file\n");
            fclose(dataFile->file);
            free(dataFile);
            return NULL;
        }
        if (consistent != CONSISTENT_MARK) {
            printf("ERROR: inconsistent file\n");
            fclose(dataFile->file);
            free(dataFile);
            return NULL;
        }
        FileRepository_goToAbsolute(dataFile, DATA_FILE_BYTE_OFFSET);
    }
    return dataFile;
}

/**
 * @brief Reads raw binary data from the file.
 *
 * Reads exactly 'count' elements, each of size 'elementSize', from the current
 * file position and updates the byte offset.
 *
 * @param dataFile: The file to read from
 * @param elementSize: Size in bytes of each element
 * @param count: Number of elements to read
 * @param buffer: Buffer to store the read data (must be large enough)
 * @return true if all data was successfully read, false otherwise
 */
bool FileRepository_read(struct DataFile *dataFile, const size_t elementSize, const size_t count, void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    const size_t bytesCount = elementSize * count;
    // If not in the data section go to it
    if (!FileRepository_goToDataSection(dataFile)) return false;
    // Check read positions to not read outside the file
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

/**
 * @brief Writes raw binary data to the file.
 *
 * Writes exactly 'count' elements, each of size 'elementSize', to the current
 * file position. Enters edit mode to mark file as inconsistent during write.
 *
 * @param dataFile: The file to write to
 * @param elementSize: Size in bytes of each element
 * @param count: Number of elements to write
 * @param buffer: The data to write (cannot be NULL)
 * @return true if all data was successfully written, false otherwise
 */
bool FileRepository_write(struct DataFile *dataFile, const size_t elementSize, const size_t count, const void *buffer) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (buffer == NULL) {
        printf("ERROR: Invalid Buffer\n");
        return false;
    }
    // If not in the data section go to it
    if (!FileRepository_goToDataSection(dataFile)) return false;
    const size_t bytesCount = elementSize * count;
    const size_t byteOffsetFinal = dataFile->byteOffset + bytesCount;
    // Check read positions to not read outside the file
    if (dataFile->size < byteOffsetFinal) dataFile->size = byteOffsetFinal;
    // Mark as inconsistent if not already
    if (!FileRepository_startEditMode(dataFile)) return false;
    if (fwrite(buffer, elementSize, count, dataFile->file) != count) {
        printf("ERROR: Failed to write data\n");
        return false;
    }
    dataFile->byteOffset += (long) bytesCount;
    return true;
}

/**
 * @brief Returns the total size of the data file in bytes (excluding metadata).
 * @param dataFile: The file to get the size of
 * @return File size in bytes, or 0 if file is invalid
 */
size_t FileRepository_fileSize(const struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return 0;
    return dataFile->size < FILE_STATUS_BYTES_COUNT ? 0 : dataFile->size - FILE_STATUS_BYTES_COUNT;
}

/**
 * @brief Moves the file cursor by a relative offset from current position.
 *
 * @param dataFile: The file to move cursor in
 * @param movement: Number of bytes to move (can be negative)
 * @return true if movement was successful, false otherwise
 */
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

/**
 * @brief Moves the file cursor at a specific byte offset in the data section from current position.
 * @param dataFile: The file to move cursor in
 * @param byteOffset: The target byte offset from start of data section
 * @return true if movement was successful, false otherwise
 */
bool FileRepository_moveUntil(struct DataFile *dataFile, const long byteOffset) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    const long absByteOffset = byteOffset + DATA_FILE_BYTE_OFFSET;
    const long movement = absByteOffset - dataFile->byteOffset;
    return FileRepository_move(dataFile, movement);
}

/**
 * @brief Positions the file cursor at a specific byte offset in the data section.
 * @param dataFile: The file to position cursor in
 * @param byteOffset: The target byte offset from start of data section
 * @return true if positioning was successful, false otherwise
 */
bool FileRepository_goTo(struct DataFile *dataFile, const long byteOffset) {
    const long absByteOffset = byteOffset + DATA_FILE_BYTE_OFFSET;
    return FileRepository_goToAbsolute(dataFile, absByteOffset);
}

/**
 * @brief Reads a boolean value from the file.
 *
 * Reads one byte and interprets it as a boolean (0 = false, non-zero = true).
 *
 * @param dataFile: The file to read from
 * @param result: Pointer to store the read boolean value
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readBool(struct DataFile *dataFile, bool *result) {
    uint8_t byte;
    if (!FileRepository_read(dataFile, UINT8_BYTES_COUNT, 1, &byte)) return false;
    // Convert from char '0' or '1' to boolean
    *result = byte - '0' ? true : false;
    return true;
}

/**
 * @brief Reads a single byte from the file.
 * @param dataFile: The file to read from
 * @param result: Pointer to store the read byte value
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readByte(struct DataFile *dataFile, uint8_t *result) {
    return FileRepository_read(dataFile, UINT8_BYTES_COUNT, 1, result);
}

/**
 * @brief Reads a 32-bit unsigned integer from the file.
 * @param dataFile: The file to read from
 * @param result: Pointer to store the read integer value
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readInt(struct DataFile *dataFile, uint32_t *result) {
    return FileRepository_read(dataFile, UINT32_BYTES_COUNT, 1, result);
}

/**
 * @brief Reads a string of specified length from the file.
 *
 * Reads exactly 'length' bytes from the file and null-terminates the result.
 *
 * @param dataFile: The file to read from
 * @param length: Number of bytes to read
 * @param result: Buffer to store the read string (must be at least length+1 bytes)
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readString(struct DataFile *dataFile, const size_t length, char *result) {
    if (!FileRepository_read(dataFile, UINT8_BYTES_COUNT, length, result)) return false;
    result[length] = '\0';
    return true;
}

/**
 * @brief Writes a boolean value to the file.
 * @param dataFile: The file to write to
 * @param value: The boolean value to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeBool(struct DataFile *dataFile, const bool value) {
    // Convert from boolean to char '0' or '1'
    const uint8_t byte = (value ? 1 : 0) + '0';
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, 1, &byte);
}

/**
 * @brief Writes a single byte to the file.
 * @param dataFile: The file to write to
 * @param value: The byte value to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeByte(struct DataFile *dataFile, const uint8_t value) {
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, 1, &value);
}

/**
 * @brief Writes a 32-bit unsigned integer to the file.
 * @param dataFile: The file to write to
 * @param value: The integer value to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeInt(struct DataFile *dataFile, const uint32_t value) {
    return FileRepository_write(dataFile, UINT32_BYTES_COUNT, 1, &value);
}

/**
 * @brief Writes a string of specified length to the file.
 * @param dataFile: The file to write to
 * @param length: Number of bytes to write
 * @param value: The string to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeString(struct DataFile *dataFile, const size_t length, const char *value) {
    if (!value) {
        printf("ERROR: Invalid String\n");
        return false;
    }
    return FileRepository_write(dataFile, UINT8_BYTES_COUNT, length, value);
}

/**
 * @brief Flushes pending changes to disk and marks file as consistent.
 * @param dataFile: The file to flush
 * @return true if flush was successful, false otherwise
 */
bool FileRepository_flush(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return false;
    if (!FileRepository_finishEditMode(dataFile)) return false;
    fflush(dataFile->file);
    return true;
}

/**
 * @brief Closes the file and frees associated resources.
 *
 * Flushes any pending changes and releases all memory allocated for the file.
 *
 * @param dataFile: The file to close
 */
void FileRepository_close(struct DataFile *dataFile) {
    if (!FileRepository_isDataFileValid(dataFile)) return;
    if (dataFile->editMode) FileRepository_flush(dataFile);
    fclose(dataFile->file);
    free(dataFile);
}
