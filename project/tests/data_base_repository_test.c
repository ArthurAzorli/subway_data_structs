#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../service/data_base_repository.h"
#include "../service/header_repository.h"

void test_database_repository() {
    const char* path = "test_database_repository.bin";
    remove(path);

    struct DataBase *db = DataBaseRepository_init(path);
    assert(db != NULL);

    // 1. Inicialização
    assert(db->dataHeader->stationsCount == 0);
    assert(db->dataHeader->pairStationsCount == 0);

    // 2. Criação de registros
    struct SubwayRecord *r1 = SubwayRecord_init();
    r1->originStationID = 1;
    r1->destinationStationID = 2;
    r1->stationName = strdup("Estacao A");
    r1->stationNameLength = strlen(r1->stationName);
    r1->lineName = strdup("Linha Azul");
    r1->lineNameLength = strlen(r1->lineName);
    assert(DataBaseRepository_createRecord(db, r1));
    assert(db->dataHeader->stationsCount == 1);
    assert(db->dataHeader->pairStationsCount == 1);

    struct SubwayRecord *r2 = SubwayRecord_init();
    r2->originStationID = 2;
    r2->destinationStationID = 1;
    r2->stationName = strdup("Estacao B");
    r2->stationNameLength = strlen(r2->stationName);
    r2->lineName = strdup("Linha Azul");
    r2->lineNameLength = strlen(r2->lineName);
    assert(DataBaseRepository_createRecord(db, r2));
    assert(db->dataHeader->stationsCount == 2);
    assert(db->dataHeader->pairStationsCount == 2);

    // 3. Leitura
    struct SubwayRecord *read1 = DataBaseRepository_readRecord(db, r1->rrn);
    assert(read1 != NULL);
    assert(strcmp(read1->stationName, "Estacao A") == 0);
    SubwayRecord_free(read1);

    // 4. Atualização de nome
    free(r1->stationName);
    r1->stationName = strdup("Estacao A+");
    r1->stationNameLength = strlen(r1->stationName);
    assert(DataBaseRepository_updateRecord(db, r1));
    struct SubwayRecord *readUpdated = DataBaseRepository_readRecord(db, r1->rrn);
    assert(strcmp(readUpdated->stationName, "Estacao A+") == 0);
    SubwayRecord_free(readUpdated);
    assert(db->dataHeader->stationsCount == 2); // A+ e B

    // 5. Atualização de par (mudando destino)
    r1->destinationStationID = 3;
    assert(DataBaseRepository_updateRecord(db, r1));
    assert(db->dataHeader->pairStationsCount == 2); // (2,1) e (1,3)

    // 6. Remoção
    assert(DataBaseRepository_deleteRecord(db, r2->rrn));
    assert(db->dataHeader->stationsCount == 1); // só A+
    assert(db->dataHeader->pairStationsCount == 1); // só (1,3)

    // 7. Reuso de espaço
    struct SubwayRecord *r3 = SubwayRecord_init();
    r3->originStationID = 3;
    r3->destinationStationID = 4;
    r3->stationName = strdup("Estacao C");
    r3->stationNameLength = strlen(r3->stationName);
    r3->lineName = strdup("Linha Verde");
    r3->lineNameLength = strlen(r3->lineName);
    assert(DataBaseRepository_createRecord(db, r3));
    assert(r3->rrn == r2->rrn); // reutilizou espaço
    assert(db->dataHeader->stationsCount == 2); // A+ e C
    assert(db->dataHeader->pairStationsCount == 2); // (1,3) e (3,4)

    // 8. Múltiplas ocorrências da mesma estação
    struct SubwayRecord *r4 = SubwayRecord_init();
    r4->originStationID = 5;
    r4->destinationStationID = 6;
    r4->stationName = strdup("Estacao C"); // mesmo nome de r3
    r4->stationNameLength = strlen(r4->stationName);
    r4->lineName = strdup("Linha Verde");
    r4->lineNameLength = strlen(r4->lineName);
    assert(DataBaseRepository_createRecord(db, r4));
    assert(db->dataHeader->stationsCount == 2); // continua 2 (A+ e C)

    assert(DataBaseRepository_deleteRecord(db, r4->rrn));
    assert(db->dataHeader->stationsCount == 2); // ainda 2, porque C continua existindo

    assert(DataBaseRepository_deleteRecord(db, r3->rrn));
    assert(db->dataHeader->stationsCount == 1); // agora só A+

    // 9. Validação de limites
    assert(DataBaseRepository_readRecord(db, 9999) == NULL); // RRN inválido

    // 10. Persistência
    DataBaseRepository_close(db);
    db = DataBaseRepository_init(path);
    assert(db->dataHeader->stationsCount == 1); // só A+
    assert(db->dataHeader->pairStationsCount == 1); // (1,3)

    DataBaseRepository_close(db);
    remove(path);

    printf("DATABASE_REPOSITORY: OK\n");
}
