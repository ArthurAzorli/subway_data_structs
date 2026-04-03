#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/file/file_repository.h"
#include "../service/database/record_repository.h"
#include "../domain/subway_record.h"

#define RECORD_LENGTH_2 80
#define HEADER_LENGTH_2 16

void record_repository_test() {
    const char *path = "test_record_repository.bin";
    remove(path);

    // 1. Criar arquivo e cabeçalho inicial (4 ints EMPTY)
    struct DataFile *df = FileRepository_openOrCreate(path);
    assert(df != NULL);
    for (int i = 0; i < 4; i++) {
        assert(FileRepository_writeInt(df, EMPTY));
    }
    FileRepository_flush(df);

    // 2. Criar registros usando SubwayRecord_init
    struct SubwayRecord *r1 = SubwayRecord_init();
    r1->rrn = 0;
    r1->originStationID = 1;
    r1->originLineID = 10;
    r1->destinationStationID = 2;
    r1->destinationDistant = 100;
    r1->interactionStationID = 30;
    r1->interactionLineID = 3;
    r1->stationNameLength = 8;
    r1->stationName = strdup("StationA");
    r1->lineNameLength = 5;
    r1->lineName = strdup("LineA");

    struct SubwayRecord *r2 = SubwayRecord_init();
    r2->rrn = 1;
    r2->originStationID = 2;
    r2->originLineID = EMPTY;
    r2->destinationStationID = EMPTY;
    r2->destinationDistant = EMPTY;
    r2->interactionStationID = EMPTY;
    r2->interactionLineID = EMPTY;
    r2->stationNameLength = 8;
    r2->stationName = strdup("StationB");
    r2->lineNameLength = 0;
    r2->lineName = NULL;

    struct SubwayRecord *r3 = SubwayRecord_init();
    r3->rrn = 2;
    r3->originStationID = 3;
    r3->originLineID = 20;
    r3->destinationStationID = 4;
    r3->destinationDistant = 200;
    r3->interactionStationID = 50;
    r3->interactionLineID = 5;
    r3->stationNameLength = 8;
    r3->stationName = strdup("StationC");
    r3->lineNameLength = 6;
    r3->lineName = strdup("LineC");

    // Inserir registros
    assert(RecordRepository_writeRecord(df, r1));
    assert(RecordRepository_writeRecord(df, r2));
    assert(RecordRepository_writeRecord(df, r3));
    FileRepository_flush(df);

    // Verificar tamanho do arquivo
    size_t size = FileRepository_fileSize(df);
    assert(size >= HEADER_LENGTH_2 + 3 * RECORD_LENGTH_2);

    // 3. Ler e verificar registros
    struct SubwayRecord *read1 = RecordRepository_readRecord(df, 0);
    assert(read1 != NULL);
    assert(read1->originStationID == 1);
    assert(read1->originLineID == 10);
    assert(read1->destinationStationID == 2);
    assert(read1->destinationDistant == 100);
    assert(read1->interactionStationID == 30);
    assert(read1->interactionLineID == 3);
    assert(read1->stationNameLength == 8);
    assert(memcmp(read1->stationName, "StationA", read1->stationNameLength) == 0);
    assert(read1->lineNameLength == 5);
    assert(memcmp(read1->lineName, "LineA", read1->lineNameLength) == 0);
    SubwayRecord_free(read1);

    struct SubwayRecord *read2 = RecordRepository_readRecord(df, 1);
    assert(read2 != NULL);
    assert(read2->originStationID == 2);
    assert(read2->originLineID == EMPTY);
    assert(read2->destinationStationID == EMPTY);
    assert(read2->destinationDistant == EMPTY);
    assert(read2->interactionStationID == EMPTY);
    assert(read2->interactionLineID == EMPTY);
    assert(read2->stationNameLength == 8);
    assert(memcmp(read2->stationName, "StationB", read2->stationNameLength) == 0);
    assert(read2->lineNameLength == 0);
    SubwayRecord_free(read2);

    struct SubwayRecord *read3 = RecordRepository_readRecord(df, 2);
    assert(read3 != NULL);
    assert(read3->originStationID == 3);
    assert(read3->originLineID == 20);
    assert(read3->destinationStationID == 4);
    assert(read3->destinationDistant == 200);
    assert(read3->interactionStationID == 50);
    assert(read3->interactionLineID == 5);
    assert(read3->stationNameLength == 8);
    assert(memcmp(read3->stationName, "StationC", read3->stationNameLength) == 0);
    assert(read3->lineNameLength == 6);
    assert(memcmp(read3->lineName, "LineC", read3->lineNameLength) == 0);
    SubwayRecord_free(read3);

    // 4. Remover múltiplos registros
    uint32_t lastRemoved = EMPTY;
    assert(RecordRepository_removeRecord(df, 1, &lastRemoved));
    assert(lastRemoved == 1);
    assert(RecordRepository_removeRecord(df, 2, &lastRemoved));
    assert(lastRemoved == 2);

    bool removed;
    assert(RecordRepository_isRemoved(df, 1, &removed));
    assert(removed == true);
    assert(RecordRepository_isRemoved(df, 2, &removed));
    assert(removed == true);

    // Verificar pilha: registro 2 aponta para 1
    uint32_t prevRemoved;
    assert(RecordRepository_readLastRemoved(df, 2, &prevRemoved));
    assert(prevRemoved == 1);

    // O topo da pilha é 2
    assert(lastRemoved == 2);

    // 5. Reabrir arquivo e verificar persistência
    FileRepository_close(df);
    df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    assert(RecordRepository_isRemoved(df, 1, &removed));
    assert(removed == true);
    assert(RecordRepository_isRemoved(df, 2, &removed));
    assert(removed == true);

    assert(RecordRepository_readLastRemoved(df, 2, &prevRemoved));
    assert(prevRemoved == 1);

    struct SubwayRecord *check1 = RecordRepository_readRecord(df, 0);
    assert(check1 != NULL);
    assert(memcmp(check1->stationName, "StationA", check1->stationNameLength) == 0);
    SubwayRecord_free(check1);

    // 6. Limpeza final
    FileRepository_close(df);
    assert(remove(path) == 0);

    SubwayRecord_free(r1);
    SubwayRecord_free(r2);
    SubwayRecord_free(r3);

    printf("RECORD_REPOSITORY: OK\n");
}
