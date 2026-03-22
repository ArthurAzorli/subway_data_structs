#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../core/file/file_repository.h"

int main() {
    const char *path = "test_datafile.bin";

    struct DataFile *df = DataFileFileRepository_openOrCreate((String)path);
    assert(df != NULL);

    // Escreve valores sequenciais
    assert(FileRepository_writeBool(df, true));
    assert(FileRepository_writeByte(df, 0xAB));
    assert(FileRepository_writeInt(df, LITTLE_ENDIAN, 123456));
    const char *msg = "Hello";
    assert(FileRepository_writeString(df, strlen(msg), msg));

    // Escreve em ambos endians
    uint32_t val = 0x12345678;
    assert(FileRepository_writeInt(df, LITTLE_ENDIAN, val));
    assert(FileRepository_writeInt(df, BIG_ENDIAN, val));

    FileRepository_close(df);

    // Reabre
    df = DataFileFileRepository_openOrCreate((String)path);
    assert(df != NULL);

    bool b;
    uint8_t by;
    uint32_t i;
    char str[16];
    uint32_t readLE, readBE;

    assert(FileRepository_readBool(df, &b));
    assert(b == true);

    assert(FileRepository_readByte(df, &by));
    assert(by == 0xAB);

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &i));
    assert(i == 123456);

    assert(FileRepository_readString(df, strlen(msg), str));
    assert(strcmp(str, "Hello") == 0);

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &readLE));
    assert(FileRepository_readInt(df, BIG_ENDIAN, &readBE));

    assert(readLE == val);
    assert(readBE == val);

    assert(FileRepository_flush(df));
    FileRepository_close(df);

    printf("Todos os testes passaram!\n");
    return 0;
}
