#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../core/file/file_repository.h"

void file_repository_test() {
    const char *path = "test_file_repository.bin";

    struct DataFile *file = FileRepository_openOrCreate(path);
    assert(file != NULL);

    // ===== ESCRITA =====
    assert(FileRepository_writeBool(file, true));
    assert(FileRepository_writeByte(file, 0xAB));
    assert(FileRepository_writeInt(file, 123456789));
    assert(FileRepository_writeString(file, 5, "HELLO"));

    assert(FileRepository_flush(file));

    size_t size = FileRepository_fileSize(file);
    assert(size > 0);

    FileRepository_close(file);

    // ===== REABERTURA =====
    file = FileRepository_openOrCreate(path);
    assert(file != NULL);

    // ===== LEITURA SEQUENCIAL =====
    bool b;
    uint8_t byte;
    uint32_t integer;
    char str[6] = {0};

    assert(FileRepository_readBool(file, &b));
    assert(b == true);

    assert(FileRepository_readByte(file, &byte));
    assert(byte == 0xAB);

    assert(FileRepository_readInt(file, &integer));
    assert(integer == 123456789);

    assert(FileRepository_readString(file, 5, str));
    assert(strcmp(str, "HELLO") == 0);

    // ===== TESTE DE NAVEGAÇÃO =====

    // bool (1) + byte (1) = offset 2
    assert(FileRepository_goTo(file, 2));

    uint32_t value2;
    assert(FileRepository_readInt(file, &value2));
    assert(value2 == 123456789);

    // voltar ao início
    assert(FileRepository_goTo(file, 0));

    // avançar 1 byte
    assert(FileRepository_move(file, 1));

    uint8_t byte2;
    assert(FileRepository_readByte(file, &byte2));
    assert(byte2 == 0xAB);

    FileRepository_close(file);

    // ===== REMOÇÃO (só acontece se nenhum assert falhou) =====
    int result = remove(path);
    assert(result == 0);
}