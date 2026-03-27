#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../core/file/file_repository.h"

void file_repository_test() {
    const char *path = "test_file_repository.bin";

    // ========================
    // CRIA E ESCREVE
    // ========================
    struct DataFile *df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    assert(FileRepository_fileSize(df) == 0);

    const char *msg = "Hello";
    uint32_t val = 0x12345678;

    assert(FileRepository_writeBool(df, true));              // offset 0
    assert(FileRepository_writeByte(df, 0xAB));              // offset 1
    assert(FileRepository_writeInt(df, LITTLE_ENDIAN, 123)); // offset 2–5
    assert(FileRepository_writeString(df, strlen(msg), msg));// offset 6–10
    assert(FileRepository_writeInt(df, LITTLE_ENDIAN, val)); // offset 11–14
    assert(FileRepository_writeInt(df, BIG_ENDIAN, val));    // offset 15–18

    size_t expectedSize = 1 + 1 + 4 + strlen(msg) + 4 + 4;
    assert(FileRepository_fileSize(df) == expectedSize);

    assert(FileRepository_flush(df));
    FileRepository_close(df);

    // ========================
    // REABRE E TESTA LEITURA
    // ========================
    df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    bool b;
    uint8_t by;
    uint32_t i, readLE, readBE;
    char str[16] = {0};

    // leitura sequencial
    assert(FileRepository_readBool(df, &b));
    assert(b == true);

    assert(FileRepository_readByte(df, &by));
    assert(by == 0xAB);

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &i));
    assert(i == 123);

    assert(FileRepository_readString(df, strlen(msg), str));
    assert(strcmp(str, msg) == 0);

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &readLE));
    assert(FileRepository_readInt(df, BIG_ENDIAN, &readBE));

    assert(readLE == val);
    assert(readBE == val);

    // ========================
    // TESTA GO TO (absoluto)
    // ========================
    assert(FileRepository_goTo(df, 2)); // volta pro int

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &i));
    assert(i == 123);

    // ========================
    // TESTA MOVE (relativo)
    // ========================
    assert(FileRepository_goTo(df, 0));
    assert(FileRepository_move(df, 2)); // pula bool + byte

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &i));
    assert(i == 123);

    // ========================
    // TESTA MOVE UNTIL (relativo até offset)
    // ========================
    assert(FileRepository_goTo(df, 0));
    assert(FileRepository_moveUntil(df, 6)); // vai até string

    memset(str, 0, sizeof(str));
    assert(FileRepository_readString(df, strlen(msg), str));
    assert(strcmp(str, msg) == 0);

    // ========================
    // TESTA FILE SIZE
    // ========================
    assert(FileRepository_fileSize(df) == expectedSize);

    // ========================
    // FECHA NO MEIO E REABRE
    // ========================
    FileRepository_close(df);

    df = FileRepository_openOrCreate(path);
    assert(df != NULL);

    // garante persistência
    assert(FileRepository_goTo(df, 11)); // posição do val LE

    assert(FileRepository_readInt(df, LITTLE_ENDIAN, &readLE));
    assert(readLE == val);

    // ========================
    // TESTE FINAL
    // ========================
    assert(FileRepository_flush(df));
    FileRepository_close(df);

    remove(path);
    printf("Todos os testes do FileRepository passaram!\n");
}