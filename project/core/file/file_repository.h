#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H
#include "../utils/types.h"

/** @brief Size in bytes for an unsigned 8-bit integer. */
#define  UINT8_BYTES_COUNT 1

/** @brief Size in bytes for an unsigned 32-bit integer. */
#define  UINT32_BYTES_COUNT 4

/** @brief Opaque structure representing an open data file. */
struct DataFile;

/**
 * @brief Opens an existing file or creates a new one if it doesn't exist.
 *
 * Attempts to open the file at the given path. If the file doesn't exist,
 * a new file is created. Verifies file consistency using a status byte.
 *
 * @param path: The file path to open or create
 * @return Pointer to DataFile structure on success, NULL on failure
 *
 * @note The returned pointer must be freed using FileRepository_close()
 */
struct DataFile *FileRepository_openOrCreate(String path);

/**
 * @brief Returns the total size of the data file in bytes (excluding metadata).
 *
 * @param dataFile: The file to get the size of
 * @return File size in bytes, or 0 if file is invalid
 *
 * @note Size excludes the 1-byte file status header
 */
size_t FileRepository_fileSize(const struct DataFile *dataFile);

/**
 * @brief Moves the file cursor by a relative offset from current position.
 *
 * @param dataFile: The file to move cursor in
 * @param movement: Number of bytes to move (can be negative)
 * @return true if movement was successful, false otherwise
 *
 * @note The movement is validated to ensure staying within file bounds
 */
bool FileRepository_move(struct DataFile *dataFile, long movement);

/**
 * @brief Moves the file cursor to an absolute offset within the data section.
 *
 * The byteOffset is relative to the start of the data section (after the 1-byte header).
 *
 * @param dataFile: The file to move cursor in
 * @param byteOffset: Absolute offset from start of data section
 * @return true if movement was successful, false otherwise
 */
bool FileRepository_moveUntil(struct DataFile *dataFile, long byteOffset);

/**
 * @brief Positions the file cursor at a specific byte offset in the data section.
 *
 * This is the primary function for repositioning within the file. The offset
 * is virtualized relative to the data section, not including the file header.
 *
 * @param dataFile: The file to position cursor in
 * @param byteOffset: The target byte offset from start of data section
 * @return true if positioning was successful, false otherwise
 *
 * @note The byteOffset parameter assumes the file header is always at position 0
 */
bool FileRepository_goTo(struct DataFile *dataFile, long byteOffset);

/**
 * @brief Reads a boolean value from the file.
 *
 * Reads one byte and interprets it as a boolean (0 = false, non-zero = true).
 *
 * @param dataFile: The file to read from
 * @param result: Pointer to store the read boolean value
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readBool(struct DataFile *dataFile, bool *result);

/**
 * @brief Reads a single byte from the file.
 *
 * @param dataFile: The file to read from
 * @param result: Pointer to store the read byte value
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readByte(struct DataFile *dataFile, uint8_t *result);

/**
 * @brief Reads a 32-bit unsigned integer from the file.
 *
 * @param dataFile: The file to read from
 * @param result: Pointer to store the read integer value
 * @return true if read was successful, false otherwise
 */
bool FileRepository_readInt(struct DataFile *dataFile, uint32_t *result);

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
bool FileRepository_readString(struct DataFile *dataFile, size_t length, char *result);

/**
 * @brief Writes a boolean value to the file.
 *
 * @param dataFile: The file to write to
 * @param value: The boolean value to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeBool(struct DataFile *dataFile, bool value);

/**
 * @brief Writes a single byte to the file.
 *
 * @param dataFile: The file to write to
 * @param value: The byte value to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeByte(struct DataFile *dataFile, uint8_t value);

/**
 * @brief Writes a 32-bit unsigned integer to the file.
 *
 * @param dataFile: The file to write to
 * @param value: The integer value to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeInt(struct DataFile *dataFile, uint32_t value);

/**
 * @brief Writes a string of specified length to the file.
 *
 * @param dataFile: The file to write to
 * @param length: Number of bytes to write
 * @param value: The string to write
 * @return true if write was successful, false otherwise
 */
bool FileRepository_writeString(struct DataFile *dataFile, size_t length, String value);

/**
 * @brief Flushes pending changes to disk and marks file as consistent.
 *
 * Must be called before closing to ensure all data is written and file status
 * is properly updated.
 *
 * @param dataFile: The file to flush
 * @return true if flush was successful, false otherwise
 */
bool FileRepository_flush(struct DataFile *dataFile);

/**
 * @brief Closes the file and frees associated resources.
 *
 * Flushes any pending changes and releases all memory allocated for the file.
 *
 * @param dataFile: The file to close (can be NULL)
 */
void FileRepository_close(struct DataFile *dataFile);


#endif //FILE_REPOSITORY_H
