//
// Created by User on 29/03/2026.

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include  "../service/data_base_repository.h"
#include "../service/header_repository.h"

void test_database_repository() {
    const char*path = "test_database_repository.bin";
    
    remove(path);

    // 1. Inicialização
    struct DataBase *db = DataBaseRepository_init(path);
    assert(db != NULL);
    assert(db->dataHeader->stationsCount == 0);
    assert(db->dataHeader->pairStationsCount == 0);
    assert(db->dataHeader->nextInsert == 0);
    assert(db->dataHeader->lastRemoved == EMPTY);

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
    assert(db->dataHeader->pairStationsCount == 0);

    struct SubwayRecord *r2 = SubwayRecord_init();
    r2->originStationID = 2;
    r2->destinationStationID = 1;
    r2->stationName = strdup("Estacao B");
    r2->stationNameLength = strlen(r2->stationName);
    r2->lineName = strdup("Linha Azul");
    r2->lineNameLength = strlen(r2->lineName);
    assert(DataBaseRepository_createRecord(db, r2));

    assert(db->dataHeader->stationsCount == 2);
    assert(db->dataHeader->pairStationsCount == 1);

    // 3. Leitura
    struct SubwayRecord *read1 = DataBaseRepository_readRecord(db, r1->rrn);
    assert(read1 != NULL);
    assert(strcmp(read1->stationName, "Estacao A") == 0);
    SubwayRecord_free(read1);

    // 4. Atualização
    r1->stationName = strdup("Estacao A+");
    r1->stationNameLength = strlen(r1->stationName);
    assert(DataBaseRepository_updateRecord(db, r1));

    struct SubwayRecord *readUpdated = DataBaseRepository_readRecord(db, r1->rrn);
    assert(strcmp(readUpdated->stationName, "Estacao A+") == 0);
    SubwayRecord_free(readUpdated);

    // 5. Remoção
    assert(DataBaseRepository_deleteRecord(db, r2->rrn));
    assert(db->dataHeader->stationsCount == 1);
    assert(db->dataHeader->pairStationsCount == 0);
    assert(db->dataHeader->lastRemoved == r2->rrn);

    // 6. Reuso de espaço
    struct SubwayRecord *r3 = SubwayRecord_init();
    r3->originStationID = 3;
    r3->destinationStationID = 4;
    r3->stationName = strdup("Estacao C");
    r3->stationNameLength = strlen(r3->stationName);
    r3->lineName = strdup("Linha Verde");
    r3->lineNameLength = strlen(r3->lineName);
    assert(DataBaseRepository_createRecord(db, r3));
    assert(r3->rrn == r2->rrn); // reutilizou espaço

    // 7. Existência
    assert(DataBaseRepository_existRecord(db, "Estacao A+"));
    assert(!DataBaseRepository_existRecord(db, "NaoExiste"));

    // 8. Fechamento e reabertura
    DataBaseRepository_close(db);
    db = DataBaseRepository_init(path);
    assert(db->dataHeader->stationsCount == 2); // A+ e C
    assert(db->dataHeader->pairStationsCount == 0);

    DataBaseRepository_close(db);
    remove(path);

    printf("DATABASE_REPOSITORY: OK\n");
}
