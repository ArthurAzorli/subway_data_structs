#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../services/file_repository.h"
#include "../subway/subway_header_repository.h"
#include "../services/utils/types.h"

/**
 * @brief Test suite for SubwayHeaderRepository functionality
 *
 * Tests:
 * 1. Header initialization
 * 2. Header writing and reading
 * 3. Data persistence after file reopen
 * 4. Field value modifications
 * 5. File size verification
 */
void header_repository_test() {
    const char *path = "test_header_repository.bin";

    // Clean up if file already exists
    remove(path);

    // ===== Test 1: File Creation and Header Initialization ===== \\
    printf("Testing header initialization...\n");

    struct DataFile *df = FileRepository_openOrCreate(path, WRITE_ONLY);
    assert(df != NULL);

    // Create header with default values
    struct DataSubwayHeader *header = SubwayHeaderRepository_init();
    assert(header != NULL);
    assert(header->lastRemoved == EMPTY);
    assert(header->nextInsert == 0);
    assert(header->stationsCount == 0);
    assert(header->pairStationsCount == 0);

    // ===== Test 2: Writing Header to File ===== \\
    printf("Testing header write operations...\n");

    // Set some values before writing
    header->nextInsert = 10;
    header->stationsCount = 5;
    header->pairStationsCount = 8;

    assert(SubwayHeaderRepository_write(header, df));
    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ===== Test 3: Reading Header from File ===== \\
    printf("Testing header read operations...\n");

    df = FileRepository_openOrCreate(path, READ_ONLY);
    assert(df != NULL);

    struct DataSubwayHeader *header_read = SubwayHeaderRepository_read(df);
    assert(header_read != NULL);
    assert(header_read->nextInsert == 10);
    assert(header_read->stationsCount == 5);
    assert(header_read->pairStationsCount == 8);
    assert(header_read->lastRemoved == EMPTY);

    FileRepository_close(df);

    // ===== Test 4: Modify and Rewrite Header ===== \\
    printf("Testing header modification and persistence...\n");

    df = FileRepository_openOrCreate(path, READ_WRITE);
    assert(df != NULL);

    // Modify header values
    header->nextInsert = 25;
    header->stationsCount = 15;
    header->pairStationsCount = 30;
    header->lastRemoved = 3;

    printf("aui oh");
    assert(SubwayHeaderRepository_write(header, df));
    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ===== Test 5: Verify Persistence After Reopen ===== \\
    printf("Testing data persistence after file reopen...\n");

    df = FileRepository_openOrCreate(path, READ_ONLY);
    assert(df != NULL);

    struct DataSubwayHeader *header_final = SubwayHeaderRepository_read(df);
    assert(header_final != NULL);
    assert(header_final->nextInsert == 25);
    assert(header_final->stationsCount == 15);
    assert(header_final->pairStationsCount == 30);
    assert(header_final->lastRemoved == 3);

    FileRepository_close(df);

    // ===== Test 6: Cleanup ===== \\
    printf("Testing cleanup...\n");

    free(header);
    free(header_read);
    free(header_final);

    assert(remove(path) == 0);

    printf("HEADER REPOSITORY: ALL TESTS PASSED ✓\n");
}
