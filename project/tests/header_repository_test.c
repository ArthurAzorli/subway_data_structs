#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../core/file/file_repository.h"
#include "../service//header_repository.h"

void header_repository_test() {
    const char* path =  "test_header_repository.bin";
    
    remove(path);

    // 1. Inicialização em arquivo vazio
    struct DataFile *df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    struct DataHeader *header = HeaderRepository_init(df);
    assert(header != NULL);
    assert(header->lastRemoved == EMPTY);
    assert(header->nextInsert == 0);
    assert(header->stationsCount == 0);
    assert(header->pairStationsCount == 0);

    size_t size = FileRepository_fileSize(df);
    assert(size >= 16); // cabeçalho ocupa 16 bytes

    FileRepository_close(df);

    // 2. Persistência após fechar e reabrir
    df = FileRepository_openOrCreate(path);
    struct DataHeader *header2 = HeaderRepository_init(df);
    assert(header2 != NULL);
    assert(header2->lastRemoved == EMPTY);
    assert(header2->nextInsert == 0);
    assert(header2->stationsCount == 0);
    assert(header2->pairStationsCount == 0);
    FileRepository_close(df);

    // 3. Alterações e salvamento
    df = FileRepository_openOrCreate(path);
    header->nextInsert = 5;
    header->stationsCount = 10;
    header->pairStationsCount = 20;
    assert(HeaderRepository_save(header, df));
    FileRepository_close(df);

    df = FileRepository_openOrCreate(path);
    struct DataHeader *header3 = HeaderRepository_init(df);
    assert(header3 != NULL);
    assert(header3->nextInsert == 5);
    assert(header3->stationsCount == 10);
    assert(header3->pairStationsCount == 20);
    FileRepository_close(df);

    // 4. Escrita de dados logo após o cabeçalho
    df = FileRepository_openOrCreate(path);
    assert(FileRepository_goTo(df, 16)); // logo após o cabeçalho
    assert(FileRepository_writeInt(df, 12345));
    FileRepository_flush(df);
    FileRepository_close(df);

    df = FileRepository_openOrCreate(path);
    int extra;
    assert(FileRepository_goTo(df, 16));
    assert(FileRepository_readInt(df, &extra));
    assert(extra == 12345);
    FileRepository_close(df);

    // Verificar tamanho do arquivo
    df = FileRepository_openOrCreate(path);
    size = FileRepository_fileSize(df);
    assert(size >= 20); // 16 bytes do cabeçalho + 4 bytes do inteiro extra
    FileRepository_close(df);

    free(header);
    free(header2);
    free(header3);
    remove(path);

    printf("HEADER_REPOSITORY: OK\n");
}