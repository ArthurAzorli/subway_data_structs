#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../core/file/file_repository.h"

void file_repository_test() {
   const char *testFile = "test_file_repository.bin";

    // 1. Criação do arquivo
    struct DataFile *df = FileRepository_openOrCreate(testFile);
    assert(df != NULL);

    // 2. Escrita de valores e verificação de tamanho
    assert(FileRepository_writeBool(df, true));
    printf("File size after writing bool: %zu\n", FileRepository_fileSize(df));
    assert(FileRepository_fileSize(df) == 1);

    assert(FileRepository_writeBool(df, false));
    assert(FileRepository_fileSize(df) == 2);

    assert(FileRepository_writeInt(df, 123456));
    assert(FileRepository_fileSize(df) == 6);

    assert(FileRepository_writeInt(df, (uint32_t)-98765)); // valor negativo
    assert(FileRepository_fileSize(df) == 10);

    assert(FileRepository_writeByte(df, 0xAB));
    assert(FileRepository_fileSize(df) == 11);

    assert(FileRepository_writeByte(df, 0x00));
    assert(FileRepository_fileSize(df) == 12);

    assert(FileRepository_writeString(df, 5, "Hello"));
    assert(FileRepository_fileSize(df) == 17);

    assert(FileRepository_writeString(df, 4, "Test"));
    assert(FileRepository_fileSize(df) == 21);

    // 3. Flush e fechamento
    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // 4. Reabrir arquivo
    df = FileRepository_openOrCreate(testFile);
    assert(df != NULL);

    // 5. Leitura dos valores
    bool bval;
    uint32_t ival;
    uint8_t byteval;
    char str[10];

    assert(FileRepository_readBool(df, &bval));
    assert(bval == true);

    assert(FileRepository_readBool(df, &bval));
    assert(bval == false);

    assert(FileRepository_readInt(df, &ival));
    assert(ival == 123456);

    assert(FileRepository_readInt(df, &ival));
    assert((int32_t)ival == -98765);

    assert(FileRepository_readByte(df, &byteval));
    assert(byteval == 0xAB);

    assert(FileRepository_readByte(df, &byteval));
    assert(byteval == 0x00);

    assert(FileRepository_readString(df, 5, str));
    assert(strcmp(str, "Hello") == 0);

    assert(FileRepository_readString(df, 4, str));
    assert(strcmp(str, "Test") == 0);

    // 6. Teste de movimentação
    assert(FileRepository_goTo(df, 0)); // volta ao início dos dados
    assert(FileRepository_move(df, 2)); // avança 2 bytes
    assert(FileRepository_moveUntil(df, 6)); // vai até posição 6 relativa

    // 7. Verificação de tamanho final
    size_t size = FileRepository_fileSize(df);
    assert(size == 21);

    // 8. Fechar e remover arquivo
    FileRepository_close(df);
    assert(remove(testFile) == 0);

    printf("Todos os testes passaram com sucesso!\n");
}