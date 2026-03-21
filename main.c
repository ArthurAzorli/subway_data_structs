#include <stdio.h>
#include "project/core/bytes/byte_array.h"


#include <stdlib.h>

#include "project/core/bytes/byte_stream.h"

// int main() {
//     // Inicialização
//     struct ByteArray *arr = ByteArray_init();
//     printf("Inicializado. Tamanho: %zu\n", ByteArray_length(arr));
//
//     // Teste pushBack
//     ByteArray_pushBack(arr, 10);
//     ByteArray_pushBack(arr, 20);
//     ByteArray_pushBack(arr, 30);
//     printf("Após pushBack: tamanho %zu, back=%d\n", ByteArray_length(arr), *ByteArray_back(arr));
//
//     // Teste pushFront
//     ByteArray_pushFront(arr, 5);
//     ByteArray_pushFront(arr, 1);
//     printf("Após pushFront: tamanho %zu, front=%d\n", ByteArray_length(arr), *ByteArray_front(arr));
//
//     // Teste get
//     for (size_t i = 0; i < ByteArray_length(arr); i++) {
//         printf("Elemento[%zu] = %d\n", i, *ByteArray_get(arr, i));
//     }
//
//     // Teste set
//     ByteArray_set(arr, 2, 99);
//     printf("Após set índice 2 = 99: elemento[2] = %d\n", *ByteArray_get(arr, 2));
//
//     // Teste setArray
//     uint8_t novos[] = {42, 43};
//     ByteArray_setArray(arr, 1, 3, novos);
//     printf("Após setArray(1..3): elemento[1]=%d, elemento[2]=%d\n",
//            *ByteArray_get(arr, 1), *ByteArray_get(arr, 2));
//
//     // Teste getArray
//     uint8_t *sub = ByteArray_getArray(arr, 1, 4);
//     printf("getArray(1..4): ");
//     for (size_t i = 0; i < 3; i++) {
//         printf("%d ", sub[i]);
//     }
//     printf("\n");
//     free(sub);
//
//     // Teste popFront
//     ByteArray_popFront(arr);
//     printf("Após popFront: tamanho %zu, front=%d\n", ByteArray_length(arr), *ByteArray_front(arr));
//
//     // Teste popBack
//     ByteArray_popBack(arr);
//     printf("Após popBack: tamanho %zu, back=%d\n", ByteArray_length(arr), *ByteArray_back(arr));
//
//     // Teste final: imprimir todos os elementos restantes
//     printf("Elementos finais:\n");
//     for (size_t i = 0; i < ByteArray_length(arr); i++) {
//         printf("Elemento[%zu] = %d\n", i, *ByteArray_get(arr, i));
//     }
//
//
//
//     // Liberação
//     ByteArray_free(arr);
//     printf("Memória liberada.\n");
//
//     return 0;
// }

#include <stdio.h>
#include <assert.h>


int main() {
    struct ByteArray *arr = ByteArray_init();

    // Teste 1: addIntegerBack (big-endian)
    ByteStream_addIntegerBack(arr, 0x628379, UINT24_BYTES_COUNT);
    assert(ByteArray_length(arr) == 3);
    assert(ByteArray_get(arr, 0) == 0x62);
    assert(ByteArray_get(arr, 1) == 0x83);
    assert(ByteArray_get(arr, 2) == 0x79);
    printf("addIntegerBack OK\n");

    // Teste 2: addIntegerFront (big-endian)
    ByteStream_addIntegerFront(arr, 0x112233, UINT24_BYTES_COUNT);
    // Agora deve estar: 0x11 0x22 0x33 0x62 0x83 0x79
    assert(ByteArray_length(arr) == 6);
    assert(ByteArray_get(arr, 0) == 0x11);
    assert(ByteArray_get(arr, 1) == 0x22);
    assert(ByteArray_get(arr, 2) == 0x33);
    printf("addIntegerFront OK\n");

    // Teste 3: setInteger (big-endian)
    ByteStream_setInteger(arr, 3, 0xAABBCC, UINT24_BYTES_COUNT);
    // Esperado: 0x11 0x22 0x33 0xAA 0xBB 0xCC
    assert(ByteArray_get(arr, 3) == 0xAA);
    assert(ByteArray_get(arr, 4) == 0xBB);
    assert(ByteArray_get(arr, 5) == 0xCC);
    printf("setInteger OK\n");

    // Teste 4: readInteger (big-endian)
    uint64_t result;
    bool ok = ByteStream_readInteger(arr, 3, UINT24_BYTES_COUNT, &result);
    assert(ok);
    assert(result == 0xAABBCC);
    printf("readInteger OK\n");

    ByteArray_free(arr);
    printf("Todos os testes passaram!\n");
    return 0;
}

