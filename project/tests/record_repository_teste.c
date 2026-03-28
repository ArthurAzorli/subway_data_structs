#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/file/file_repository.h"
#include "../service/record_repository.h"
#include "../domain/subway_record.h"

void record_repository_test() {
    const char *path = "test_record_repository.bin";

    // limpa arquivo anterior
    remove(path);

    struct DataFile *df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    FileRepository_writeInt(df, -1);
    FileRepository_writeInt(df, -1);
    FileRepository_writeInt(df, -1);
    FileRepository_writeInt(df, -1);

    // ========================
    // CRIA RECORDS
    // ========================
    struct SubwayRecord *r1 = SubwayRecord_init();
    struct SubwayRecord *r2 = SubwayRecord_init();

    assert(r1 && r2);

    // preenchimento manual
    r1->rrn = 0;
    r1->originStationID = 1;
    r1->originLineID = 10;
    r1->destinationStationID = 2;
    r1->destinationDistant = 100;
    r1->interactionLineID = 20;
    r1->interactionStationID = 3;
    r1->stationName = strdup("Station A");
    r1->stationNameLength = strlen(r1->stationName);
    r1->lineName = strdup("Line X");
    r1->lineNameLength = strlen(r1->lineName);

    r2->rrn = 1;
    r2->originStationID = 5;
    r2->originLineID = 50;
    r2->destinationStationID = 6;
    r2->destinationDistant = 200;
    r2->interactionLineID = 60;
    r2->interactionStationID = 7;
    r2->stationName = strdup("Station B");
    r2->stationNameLength = strlen(r2->stationName);
    r2->lineName = strdup("Line Y");
    r2->lineNameLength = strlen(r2->lineName);

    // ========================
    // ESCREVE RECORDS
    // ========================
    assert(RecordRepository_writeRecord(df, r1));
    assert(RecordRepository_writeRecord(df, r2));

    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ========================
    // REABRE E TESTA LEITURA
    // ========================
    df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    struct SubwayRecord *read1 = RecordRepository_readRecord(df, 0);
    struct SubwayRecord *read2 = RecordRepository_readRecord(df, 1);

    assert(read1 != NULL);
    assert(read2 != NULL);

    assert(strcmp(read1->stationName, "Station A") == 0);
    assert(strcmp(read2->stationName, "Station B") == 0);

    // ========================
    // TESTA isRemoved
    // ========================
    bool removed;

    assert(RecordRepository_isRemoved(df, 0, &removed));
    assert(removed == false);

    // ========================
    // TESTA REMOÇÃO
    // ========================
    uint32_t lastRemoved = -1;

    assert(RecordRepository_removeRecord(df, 0, &lastRemoved));
    assert(lastRemoved == 0);

    assert(RecordRepository_isRemoved(df, 0, &removed));
    assert(removed == true);

    // ========================
    // TESTA readLastRemoved
    // ========================
    uint32_t nextRemoved;

    assert(RecordRepository_readLastRemoved(df, 0, &nextRemoved));
    assert(nextRemoved == -1); // primeiro removido aponta pra EMPTY

    // ========================
    // REMOVE SEGUNDO (encadeamento)
    // ========================
    assert(RecordRepository_removeRecord(df, 1, &lastRemoved));
    assert(lastRemoved == 1);

    assert(RecordRepository_readLastRemoved(df, 1, &nextRemoved));
    assert(nextRemoved == 0);

    // ========================
    // FECHA E REABRE
    // ========================
    FileRepository_close(df);

    df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    // registros removidos não devem ser lidos
    struct SubwayRecord *shouldBeNull = RecordRepository_readRecord(df, 0);
    assert(shouldBeNull == NULL);

    // ========================
    // CLEANUP
    // ========================
    FileRepository_close(df);

    SubwayRecord_free(r1);
    SubwayRecord_free(r2);
    SubwayRecord_free(read1);
    SubwayRecord_free(read2);

    remove(path);

    printf("✅ Todos os testes do RecordRepository passaram!\n");
}