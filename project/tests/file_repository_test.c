#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../services/file/file_repository.h"

/**
 * @brief Test suite for FileRepository functionality
 *
 * Tests basic file operations:
 * 1. File creation in WRITE mode
 * 2. Writing data (boolean, integers, bytes, strings)
 * 3. File closure and flushing
 * 4. File reopening in READ mode
 * 5. Reading data back and verifying integrity
 * 6. File navigation
 */
void file_repository_test() {
    const char *path = "test_file_repository.bin";

    // Clean up if file already exists
    remove(path);

    // ===== Test 1: File Creation and Writing ===== \\
    printf("Testing file creation and write operations...\n");

    struct DataFile *df = FileRepository_openOrCreate(path, WRITE_ONLY);
    assert(df != NULL);

    // Write boolean values (using BOOLEAN type)
    bool bval_true = true;
    assert(FileRepository_write(df, BOOLEAN, &bval_true, 1));

    bool bval_false = false;
    assert(FileRepository_write(df, BOOLEAN, &bval_false, 1));

    // Write 32-bit integers
    uint32_t int1 = 123456;
    assert(FileRepository_write(df, INTEGER, &int1, 1));

    uint32_t int2 = (uint32_t)-98765;
    assert(FileRepository_write(df, INTEGER, &int2, 1));

    // Write individual bytes
    uint8_t byte1 = 0xAB;
    assert(FileRepository_write(df, CHAR, &byte1, 1));

    uint8_t byte2 = 0x00;
    assert(FileRepository_write(df, CHAR, &byte2, 1));

    // Write strings
    assert(FileRepository_write(df, CHAR, "Hello", 5));
    assert(FileRepository_write(df, CHAR, "Test", 4));

    // ===== Test 2: Flush and Close ===== \\
    printf("Testing flush and file closure...\n");

    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ===== Test 3: Reopen and Read ===== \\
    printf("Testing file reopen and read operations...\n");

    df = FileRepository_openOrCreate(path, READ_ONLY);
    assert(df != NULL);

    // Read and verify boolean values
    bool bval_read;
    assert(FileRepository_read(df, BOOLEAN, &bval_read, 1));
    assert(bval_read == true);

    assert(FileRepository_read(df, BOOLEAN, &bval_read, 1));
    assert(bval_read == false);

    // Read and verify integers
    uint32_t ival;
    assert(FileRepository_read(df, INTEGER, &ival, 1));
    assert(ival == 123456);

    assert(FileRepository_read(df, INTEGER, &ival, 1));
    assert((int32_t)ival == -98765);

    // Read and verify bytes
    uint8_t byteval;
    assert(FileRepository_read(df, CHAR, &byteval, 1));
    assert(byteval == 0xAB);

    assert(FileRepository_read(df, CHAR, &byteval, 1));
    assert(byteval == 0x00);

    // Read and verify strings
    char str[10];
    assert(FileRepository_read(df, CHAR, str, 5));
    str[5] = '\0';
    assert(strcmp(str, "Hello") == 0);

    assert(FileRepository_read(df, CHAR, str, 4));
    str[4] = '\0';
    assert(strcmp(str, "Test") == 0);

    // ===== Test 4: File Navigation ===== \\
    printf("Testing file navigation...\n");

    // Go to beginning of data (offset 0)
    FileRepository_goto(df, 0);

    // Move forward 2 bytes (skip first two boolean values)
    FileRepository_move(df, 2);

    // ===== Test 5: Cleanup ===== \\
    printf("Testing cleanup...\n");

    FileRepository_close(df);
    assert(remove(path) == 0);

    printf("FILE REPOSITORY: ALL TESTS PASSED ✓\n");
}

