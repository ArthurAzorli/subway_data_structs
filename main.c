#include <stdio.h>

#include "project/program.h"
#include "project/config.h"
#include "project/lib/provided.h"
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


void showMenu() {
#ifdef SHOW_MENU
#if SHOW_MENU
    printf("\n");
    printf("=============================\n");
    printf("             Menu            \n");
    printf("=============================\n");
    printf("\n");
    printf("1. Start Session\n");
    printf("0. Exit\n");
    printf("\n");
    printf("Enter a option:\n");
#endif
#endif
}

int main() {
    runTests();
    struct Session *session = Session_init();
    if (session == NULL) {
        throwError("Could not initialize session");
        return 1;
    }

    int option;
    do {
        showMenu();
        if (scanf("%d", &option) != 1) {
            throwError("Incorrect optional input");
            continue;
        }
        switch (option) {
            case 1:
                if (!Program_startSession(session)) {
                    throwError("Could not start session");
                    Session_clear(session);
                    return 1;
                }
                break;
            case 0:
                continue;
            default:
                throwError("Unrecognized option");
                continue;
        }

        if (session->outputFilePath != NULL) BinarioNaTela(session->outputFilePath);
    } while (option != 0);
    Session_clear(session);
    return 0;
}
