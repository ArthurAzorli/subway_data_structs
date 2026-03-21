#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../core/file/file_repository.h"

int main() {
    const char *path = "test_datafile.bin";

    // 1. Cria ou abre arquivo
    struct DataFile *df = DataFileFileRepository_openOrCreate((String)path);
    assert(df != NULL);

    // 2. Escreve valores
    assert(FileRepository_writeBool(df, 1, true));
    assert(FileRepository_writeByte(df, 2, 0xAB));
    assert(FileRepository_writeInt(df, 3, 123456));
    const char *msg = "Hello";
    assert(FileRepository_writeString(df, 7, strlen(msg), msg));

    // 3. Fecha no meio da operação
    FileRepository_close(df);

    // 4. Reabre e confirma leitura
    df = DataFileFileRepository_openOrCreate((String)path);
    assert(df != NULL);

    bool b;
    uint8_t by;
    uint32_t i;
    char str[16];

    assert(FileRepository_readBool(df, 1, &b));
    assert(b == true);

    assert(FileRepository_readByte(df, 2, &by));
    assert(by == 0xAB);

    assert(FileRepository_readInt(df, 3, &i));
    assert(i == 123456);

    assert(FileRepository_readString(df, 7, strlen(msg), str));
    assert(strcmp(str, "Hello") == 0);

    // 5. Flush e fecha
    assert(FileRepository_flush(df));
    FileRepository_close(df);

    printf("Todos os testes passaram!\n");
    return 0;
}
