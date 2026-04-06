// Arthur Gagliardi Azorli - 16855452
// Nícolas Silva Scorza - 17025079

/**
 * @brief Application entry point and main event loop.
 *
 * Initializes tests if configured, displays user menu, and handles user
 * selections to invoke various program operations.
 */

#include <stdio.h>

#include "project/program.h"
#include "project/config.h"
#include "project/core/utils/errors.h"

#ifdef RUN_TESTS
#if RUN_TESTS
#include "project/tests/string_test.c"
#include "project/tests/file_repository_test.c"
#include "project/tests/header_repository_test.c"
#include "project/tests/record_repository_test.c"
#include "project/tests/data_base_repository_test.c"
#include "project/tests/input_repository_test.c"
#endif
#endif


/**
 * @brief Runs all configured unit tests.
 *
 * Executes test suites for string utilities, file operations, header management,
 * record I/O, database operations, and input parsing. Only runs if RUN_TESTS
 * configuration is enabled.
 */
void runTests() {
#ifdef RUN_TESTS
#if RUN_TESTS
    printf("Starting tests...\n");
    string_test();
    file_repository_test();
    record_repository_test();
    header_repository_test();
    database_repository_test();
    input_repository_test();
    printf("Finish tests\n");
#endif
#endif
}

/**
 * @brief Displays the main menu to the user.
 *
 * Shows available program operations and prompts for user selection.
 * Menu display is controlled by SHOW_MENU configuration option.
 */
void showMenu() {
#ifdef SHOW_MENU
#if SHOW_MENU
    printf("\n");
    printf("=============================\n");
    printf("             Menu            \n");
    printf("=============================\n");
    printf("\n");
    printf("1. Start Session\n");
    printf("2. Show Records\n");
    printf("3. Search Records\n");
    printf("4. Get Record by RRN\n");
    printf("0. Exit\n");
    printf("\n");
    printf("Enter a option:\n");
#endif
#endif
}

void printMessageError() {
    printf("Falha no processamento do arquivo.\n");
}

/**
 * @brief Main entry point for the subway database application.
 *
 * Runs configured unit tests, displays the main menu, and processes user
 * commands in a loop until the user chooses to exit.
 *
 * @return EXIT_SUCCESS (0) on successful completion, non-zero on error
 */
int main() {
    runTests();
    int option;
    do {
        showMenu();
        if (scanf("%d", &option) != 1) {
            throwError("Incorrect optional input");
            continue;
        }
        switch (option) {
            case 1: {
                if (!Program_readFromFile()) {
                    printMessageError();
                    return 0;
                }
                break;
            }
            case 2: {
                if (!Program_showRecords()) {
                    printMessageError();
                    return 0;
                }
                break;
            }
            case 3: {
                if (!Program_searchRecord()) {
                    printMessageError();
                    return 0;
                }
                break;
            }
            case 4: {
                if (!Program_getRecordByRRN()) {
                    printMessageError();
                    return 0;
                }
                break;
            }
            case 0:
                return 0;
            default:
                throwError("Unrecognized option");
        }
    } while (LOOP_RUN);
    return 0;
}
