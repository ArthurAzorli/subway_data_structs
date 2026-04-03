#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../service/input/input_repository.h"

void input_repository_test() {
    const char *path = "test_input_repository.csv";

    // 1. Criar arquivo CSV de teste
    FILE *f = fopen(path, "w");
    assert(f != NULL);
    fprintf(f, "CodEstacao,NomeEstacao,CodLinha,NomeLinha,CodProxEst,DistanciaProxEst,CodLinhaInteg,CodEstacaoInteg\n");
    fprintf(f, "1,Tucuruvi,1,Azul,2,992,,\n");
    fprintf(f, "2,Parada Inglesa,1,Azul,3,1057,,\n");
    fprintf(f, "3,Jardim Sao Paulo,1,Azul,4,1474,,\n");
    // linha com valores nulos em campos opcionais
    fprintf(f, "4,Santana,1,Azul,,,,\n");
    fclose(f);

    // 2. Abrir arquivo com InputRepository
    struct InputFile *inputFile = InputRepository_openFile(path);
    assert(inputFile != NULL);

    // 3. Ler registros
    struct SubwayRecord *r1 = InputRepository_extractRecord(inputFile);
    assert(r1 != NULL);
    assert(r1->originStationID == 1);
    assert(strcmp(r1->stationName, "Tucuruvi") == 0);
    assert(r1->originLineID == 1);
    assert(strcmp(r1->lineName, "Azul") == 0);
    assert(r1->destinationStationID == 2);
    assert(r1->destinationDistant == 992);
    SubwayRecord_free(r1);

    struct SubwayRecord *r2 = InputRepository_extractRecord(inputFile);
    assert(r2 != NULL);
    assert(r2->originStationID == 2);
    assert(strcmp(r2->stationName, "Parada Inglesa") == 0);
    assert(r2->destinationStationID == 3);
    assert(r2->destinationDistant == 1057);
    SubwayRecord_free(r2);

    struct SubwayRecord *r3 = InputRepository_extractRecord(inputFile);
    assert(r3 != NULL);
    assert(r3->originStationID == 3);
    assert(strcmp(r3->stationName, "Jardim Sao Paulo") == 0);
    assert(r3->destinationStationID == 4);
    assert(r3->destinationDistant == 1474);
    SubwayRecord_free(r3);

    struct SubwayRecord *r4 = InputRepository_extractRecord(inputFile);
    assert(r4 != NULL);
    assert(r4->originStationID == 4);
    assert(strcmp(r4->stationName, "Santana") == 0);
    // campos opcionais nulos
    assert(r4->destinationStationID == EMPTY);
    assert(r4->destinationDistant == EMPTY);
    SubwayRecord_free(r4);

    // 4. Fechar arquivo
    InputRepository_closeFile(inputFile);

    // 5. Remover arquivo de teste
    remove(path);

    printf("INPUT_REPOSITORY: OK\n");
}
