#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "project/service/data_base_repository.h"
#include "project/domain/subway_line_record.h"

int main() {
    struct DataBase *db = DataBaseRepository_init("test.bin");
    assert(db != NULL);

    // Caso 1: Registro válido
    struct SubwayLineRecord *rec1 = SubwayLineRecord_init();
    rec1->originStationID = 1;
    rec1->stationName = strdup("Estacao Central");
    rec1->stationNameLength = strlen(rec1->stationName);
    assert(DataBaseRepository_createRecord(db, rec1) == true);

    // Deve existir
    assert(DataBaseRepository_existRecord(db, "Estacao Central") == true);

    // Caso 2: Registro inválido (stationName nulo)
    struct SubwayLineRecord *rec2 = SubwayLineRecord_init();
    rec2->originStationID = 2;
    rec2->stationName = NULL;
    rec2->stationNameLength = 0;
    assert(DataBaseRepository_createRecord(db, rec2) == false);
    SubwayLineRecord_free(rec2);

    // Caso 3: Registro inválido (originStationID vazio)
    struct SubwayLineRecord *rec3 = SubwayLineRecord_init();
    rec3->originStationID = EMPTY;
    rec3->stationName = strdup("Estacao Fantasma");
    rec3->stationNameLength = strlen(rec3->stationName);
    assert(DataBaseRepository_createRecord(db, rec3) == false);
    SubwayLineRecord_free(rec3);

    // Caso 4: Truncamento de string
    char longName[200];
    memset(longName, 'A', sizeof(longName)-1);
    longName[sizeof(longName)-1] = '\0';

    struct SubwayLineRecord *rec4 = SubwayLineRecord_init();
    rec4->originStationID = 3;
    rec4->stationName = strdup(longName);
    rec4->stationNameLength = strlen(rec4->stationName);
    assert(DataBaseRepository_createRecord(db, rec4) == true);

    // Deve existir mesmo truncado
    assert(DataBaseRepository_existRecord(db, rec4->stationName) == true);

    // Caso 5: Update de registro
    struct SubwayLineRecord *recUpdate = SubwayLineRecord_init();
    recUpdate->rrn = rec1->rrn; // rrn interno, obtido do rec1
    recUpdate->originStationID = 1;
    recUpdate->stationName = strdup("Estacao Alterada");
    recUpdate->stationNameLength = strlen(recUpdate->stationName);
    assert(DataBaseRepository_updateRecord(db, recUpdate) == true);

    // Deve existir com novo nome
    assert(DataBaseRepository_existRecord(db, "Estacao Alterada") == true);

    // Caso 6: Delete de registro
    assert(DataBaseRepository_deleteRecord(db, rec1->rrn) == true);
    assert(DataBaseRepository_existRecord(db, "Estacao Alterada") == false);

    DataBaseRepository_close(db);
    printf("Todos os testes passaram!\n");
    return 0;
}