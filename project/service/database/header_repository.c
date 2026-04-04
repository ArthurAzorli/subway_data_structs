#include "header_repository.h"
#include "../../core/utils/errors.h"

#include <stdlib.h>

#define HEADER_FILE_LENGTH 16
#define HEADER_FILE_SECTION_OFFSET 0

/**
 * @brief Validates that a DataHeader structure contains valid data.
 *
 * Checks that the header pointer is not NULL and that all required fields
 * have been properly initialized (none set to EMPTY value).
 *
 * @param header: The header to validate
 * @return true if header is valid, false otherwise
 */
bool HeaderRepository_isHeaderValid(const struct DataHeader *header) {
    if (header == NULL) return false;
    if (header->pairStationsCount == EMPTY) return false;
    if (header->stationsCount == EMPTY) return false;
    if (header->nextInsert == EMPTY) return false;
    return true;
}

/**
 * @brief Writes the header structure to the data file.
 *
 * Sequentially writes all four uint32_t fields of the header to the file
 * at the current file position.
 *
 * @param header: The header to write (must be valid)
 * @param dataFile: Open data file to write to
 * @return true if all fields were successfully written, false otherwise
 */
bool HeaderRepository_writeHeader(const struct DataHeader *header, struct DataFile *dataFile) {
    if (header == NULL || dataFile == NULL) return false;
    if (!FileRepository_writeInt(dataFile, header->lastRemoved)) return false;
    if (!FileRepository_writeInt(dataFile, header->nextInsert)) return false;
    if (!FileRepository_writeInt(dataFile, header->stationsCount)) return false;
    if (!FileRepository_writeInt(dataFile, header->pairStationsCount)) return false;
    return true;
}

/**
 * @brief Reads the header structure from the data file.
 *
 * Sequentially reads all four uint32_t fields from the file at the current
 * file position and populates the header structure.
 *
 * @param header: Pointer to header structure to populate (must be allocated)
 * @param dataFile: Open data file to read from
 * @return true if all fields were successfully read, false otherwise
 */
bool HeaderRepository_readHeader(struct DataHeader *header, struct DataFile *dataFile) {
    if (dataFile == NULL) return false;
    if (!FileRepository_readInt(dataFile, &header->lastRemoved)) return false;
    if (!FileRepository_readInt(dataFile, &header->nextInsert)) return false;
    if (!FileRepository_readInt(dataFile, &header->stationsCount)) return false;
    if (!FileRepository_readInt(dataFile, &header->pairStationsCount)) return false;
    return true;
}


/**
 * @brief Initializes the database header from file or creates a new one.
 *
 * Reads the header from the beginning of the data file. If the file is too small
 * to contain a complete header, initializes a new one with default values and saves
 * it to the file.
 *
 * @param dataFile: Open data file to read/write header from
 * @return Pointer to allocated DataHeader on success, NULL on failure
 *
 * @note The returned pointer must be freed by the caller
 */
struct DataHeader *HeaderRepository_init(struct DataFile *dataFile) {
    if (dataFile == NULL) {
        throwError("Invalid data file");
        return NULL;
    }

    if (!FileRepository_goTo(dataFile, HEADER_FILE_SECTION_OFFSET)) return NULL;
    struct DataHeader *header = malloc(sizeof(struct DataHeader));
    if (header == NULL) {
        throwError("Could not allocate memory for the header");
        return NULL;
    }

    header->lastRemoved = EMPTY;
    header->nextInsert = 0;
    header->stationsCount = 0;
    header->pairStationsCount = 0;

    //se nao houver um header ou um incompleto o reseta, se nao le o header anetrior
    const size_t fileSize = FileRepository_fileSize(dataFile);
    if (fileSize < HEADER_FILE_LENGTH) {
        if (!HeaderRepository_save(header, dataFile)) {
            free(header);
            return NULL;
        }
    } else {
        if (!HeaderRepository_readHeader(header, dataFile)) {
            free(header);
            return NULL;
        }
    }
    return header;
}

/**
 * @brief Saves the database header to file.
 *
 * Writes the header structure to the beginning of the file. The header is always
 * written at byte offset 0 of the data section. Validates the header before writing
 * and marks the file as being in edit mode.
 *
 * @param header: The header to save (must be valid)
 * @param dataFile: Open data file to write header to
 * @return true if header was successfully saved, false otherwise
 */
bool HeaderRepository_save(const struct DataHeader *header, struct DataFile *dataFile) {
    if (!HeaderRepository_isHeaderValid(header)) {
        throwError("Invalid header file");
        return false;
    }
    if (dataFile == NULL) {
        throwError("Invalid data file");
        return false;
    }
    if (!FileRepository_goTo(dataFile, HEADER_FILE_SECTION_OFFSET)) return false;
    if (!HeaderRepository_writeHeader(header, dataFile)) {
        throwError("Could not write header");
        return false;
    }
    return true;
}
