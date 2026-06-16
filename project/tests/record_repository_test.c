#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../services/file_repository.h"
#include "../subway/subway_record_repository.h"
#include "../services/types.h"

#define RECORD_LENGTH 80

/**
 * @brief Test suite for SubwayRecordRepository functionality
 *
 * Tests:
 * 1. Record initialization
 * 2. Record writing to file
 * 3. Record reading from file
 * 4. Data persistence and integrity
 * 5. Multiple records handling
 * 6. Record removal
 * 7. Memory management
 */
void record_repository_test() {
    const char *path = "test_record_repository.bin";
    remove(path);

    // ===== Test 1: File Creation and Header Setup ===== \\
    printf("Testing file creation and header setup...\n");

    struct DataFile *df = FileRepository_openOrCreate(path, WRITE_ONLY);
    assert(df != NULL);

    // Write header (4 empty integers)
    uint32_t header_val = EMPTY;
    assert(FileRepository_write(df, INTEGER, &header_val, 1));
    assert(FileRepository_write(df, INTEGER, &header_val, 1));
    assert(FileRepository_write(df, INTEGER, &header_val, 1));
    assert(FileRepository_write(df, INTEGER, &header_val, 1));

    // ===== Test 2: Create Records with Different Data ===== \\
    printf("Testing record creation and writing...\n");

    // Record 1: Full data with both strings
    struct SubwayRecord *r1 = SubwayRecord_init();
    assert(r1 != NULL);
    r1->originStationID = 100;
    r1->originLineID = 10;
    r1->destinationStationID = 200;
    r1->destinationDistant = 5000;
    r1->interactionLineID = 20;
    r1->interactionStationID = 150;
    r1->stationNameLength = 7;
    r1->stationName = malloc(8);
    strcpy(r1->stationName, "Station");
    r1->lineNameLength = 4;
    r1->lineName = malloc(5);
    strcpy(r1->lineName, "Line");

    assert(SubwayRecordRepository_writeRecord(df, r1));

    // Record 2: Partial data (some EMPTY fields)
    struct SubwayRecord *r2 = SubwayRecord_init();
    assert(r2 != NULL);
    r2->originStationID = 200;
    r2->originLineID = 20;
    r2->destinationStationID = 300;
    r2->destinationDistant = EMPTY;
    r2->interactionLineID = EMPTY;
    r2->interactionStationID = EMPTY;
    r2->stationNameLength = 6;
    r2->stationName = malloc(7);
    strcpy(r2->stationName, "Center");
    r2->lineNameLength = 5;
    r2->lineName = malloc(6);
    strcpy(r2->lineName, "Blue");

    assert(SubwayRecordRepository_writeRecord(df, r2));

    // Record 3: Minimal data
    struct SubwayRecord *r3 = SubwayRecord_init();
    assert(r3 != NULL);
    r3->originStationID = 300;
    r3->originLineID = 30;
    r3->destinationStationID = EMPTY;
    r3->destinationDistant = EMPTY;
    r3->interactionLineID = EMPTY;
    r3->interactionStationID = EMPTY;
    r3->stationNameLength = 5;
    r3->stationName = malloc(6);
    strcpy(r3->stationName, "South");
    r3->lineNameLength = 0;
    r3->lineName = NULL;

    assert(SubwayRecordRepository_writeRecord(df, r3));

    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ===== Test 3: Reopen File and Read Records ===== \\
    printf("Testing record reading and data verification...\n");

    df = FileRepository_openOrCreate(path, READ_ONLY);
    assert(df != NULL);

    // Skip header (16 bytes)
    FileRepository_move(df, 16);

    // Read Record 1
    struct SubwayRecord *read1 = SubwayRecordRepository_readRecord(df);
    printf("Read Record 1: originStationID=%u, stationName=%s\n", read1->originStationID, read1->stationName);
    assert(read1 != NULL);
    assert(read1->originStationID == 100);
    assert(read1->originLineID == 10);
    assert(read1->destinationStationID == 200);
    assert(read1->destinationDistant == 5000);
    assert(read1->interactionLineID == 20);
    assert(read1->interactionStationID == 150);
    assert(read1->stationNameLength == 7);
    assert(strcmp(read1->stationName, "Station") == 0);
    assert(read1->lineNameLength == 4);
    assert(strcmp(read1->lineName, "Line") == 0);

    // Read Record 2
    struct SubwayRecord *read2 = SubwayRecordRepository_readRecord(df);
    assert(read2 != NULL);
    assert(read2->originStationID == 200);
    assert(read2->originLineID == 20);
    assert(read2->destinationStationID == 300);
    assert(read2->destinationDistant == EMPTY);
    assert(read2->interactionLineID == EMPTY);
    assert(read2->interactionStationID == EMPTY);
    assert(read2->stationNameLength == 6);
    assert(strcmp(read2->stationName, "Center") == 0);
    assert(read2->lineNameLength == 5);
    assert(strcmp(read2->lineName, "Blue") == 0);

    // Read Record 3
    struct SubwayRecord *read3 = SubwayRecordRepository_readRecord(df);
    assert(read3 != NULL);
    assert(read3->originStationID == 300);
    assert(read3->originLineID == 30);
    assert(read3->destinationStationID == EMPTY);
    assert(read3->destinationDistant == EMPTY);
    assert(read3->interactionLineID == EMPTY);
    assert(read3->interactionStationID == EMPTY);
    assert(read3->stationNameLength == 5);
    assert(strcmp(read3->stationName, "South") == 0);
    assert(read3->lineNameLength == 0);

    // ===== Test 4: Test Record Removal ===== \\
    printf("Testing record removal...\n");

    FileRepository_close(df);

    df = FileRepository_openOrCreate(path, READ_WRITE);
    assert(df != NULL);

    // Move to second record and remove it
    FileRepository_move(df, 16 + RECORD_LENGTH); // Skip header and first record
    assert(SubwayRecordRepository_removeRecord(df, EMPTY));

    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ===== Test 5: Verify Removed Record ===== \\
    printf("Testing removed record verification...\n");

    df = FileRepository_openOrCreate(path, READ_ONLY);
    assert(df != NULL);

    // Skip header and first record
    FileRepository_move(df, 16 + RECORD_LENGTH);

    // Try to read removed record (should return NULL)
    struct SubwayRecord *removed_read = SubwayRecordRepository_readRecord(df);
    assert(removed_read == NULL);

    FileRepository_close(df);

    // ===== Test 6: Memory Cleanup ===== \\
    printf("Testing memory cleanup...\n");

    SubwayRecord_free(r1);
    SubwayRecord_free(r2);
    SubwayRecord_free(r3);
    SubwayRecord_free(read1);
    SubwayRecord_free(read2);
    SubwayRecord_free(read3);

    // ===== Test 7: File Cleanup ===== \\
    printf("Testing file cleanup...\n");

    assert(remove(path) == 0);

    printf("RECORD REPOSITORY: ALL TESTS PASSED ✓\n");
}


