// #include "project/tests/file_repository_test.c"
// #include "project/tests/record_repository_test.c"
// #include "project/tests/header_repository_test.c"
// #include "project/tests/data_base_repository_test.c"
// #include "project/tests/string_test.c"

#include <stdio.h>
#include <stdlib.h>
#include "project/core/file/file_repository.h"
#include "project/lib/provided.h"
#include "project/core/utils/string.h"
#include "project/core/utils/types.h"
#include "project/domain/subway_record.h"
#include "project/service/data_base_repository.h"
#include "project/service/header_repository.h"
#include "project/service/record_repository.h"


int main() {
    // string_test();
    // file_repository_test();
    // record_repository_test();
    // header_repository_test();
    // test_database_repository();

    char entrada[101], saida[101];
    struct DataBase *dataBase;

    int input;
    if (scanf("%d", &input) != 1) {
        printf("Incorrect input\n");
        return 0;
    }

    switch (input) {
        case 1:
            if (scanf("%s %s", entrada, saida) != 2) {
                return 0;
            }
            FILE *csvFile = fopen(entrada, "r");
            if (csvFile == NULL) {
                return 1;
            }
            dataBase = DataBaseRepository_init(saida);
            if (dataBase == NULL) {
                return 1;
            }

            char linha[1024];
            char *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8;

            fgets(linha, sizeof(linha), csvFile);
            while (fgets(linha, sizeof(linha), csvFile) != NULL) {
                linha[strcspn(linha, "\r\n")] = 0;

                split(linha, ",", &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8);

                struct SubwayRecord *rec = SubwayRecord_init();

                rec->originStationID = (c1 && strlen(c1) > 0) ? (uint32_t) atoi(c1) : EMPTY;
                rec->originLineID = (c3 && strlen(c3) > 0) ? (uint32_t) atoi(c3) : EMPTY;
                rec->destinationStationID = (c5 && strlen(c5) > 0) ? (uint32_t) atoi(c5) : EMPTY;
                rec->destinationDistant = (c6 && strlen(c6) > 0) ? (uint32_t) atoi(c6) : EMPTY;
                rec->interactionLineID = (c7 && strlen(c7) > 0) ? (uint32_t) atoi(c7) : EMPTY;
                rec->interactionStationID = (c8 && strlen(c8) > 0) ? (uint32_t) atoi(c8) : EMPTY;

                if (c2 && strlen(c2) > 0) {
                    rec->stationName = c2;
                    rec->stationNameLength = (uint32_t) strlen(c2);
                } else {
                    rec->stationName = NULL;
                    rec->stationNameLength = 0;
                    free(c2);
                }

                if (c4 && strlen(c4) > 0) {
                    rec->lineName = c4;
                    rec->lineNameLength = (uint32_t) strlen(c4);
                } else {
                    rec->lineName = NULL;
                    rec->lineNameLength = 0;
                    free(c4);
                }

                DataBaseRepository_createRecord(dataBase, rec);

                free(c1);
                free(c3);
                free(c5);
                free(c6);
                free(c7);
                free(c8);

                SubwayRecord_free(rec);
            }

            fclose(csvFile);
            DataBaseRepository_close(dataBase);

            BinarioNaTela(saida);

            break;
    }

    return 0;
}
