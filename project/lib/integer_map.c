#include "integer_map.h"
#include <stdlib.h>
#include <stdbool.h>

// Capacidade inicial do mapa e incremento quando atinge limite
#define INITIAL_CAPACITY 100
#define CAPACITY_INCREMENT 100

/**
 * @brief Estrutura interna representando um par chave-valor
 * @note 'active' marca se a entrada está sendo usada ou foi deletada
 */
typedef struct {
    uint32_t key;
    uint32_t value;
    bool active;
} IntegerMapEntry;

/**
 * @brief Estrutura do mapa inteiro
 * @note Usa array dinâmico de pares chave-valor com crescimento automático
 */
struct IntegerMap {
    IntegerMapEntry *entries;  // Array dinâmico de pares chave-valor
    size_t size;               // Número de elementos ativos atualmente
    size_t capacity;           // Capacidade alocada no momento
};

/**
 * @brief Cria um novo mapa inteiro vazio
 * @return Ponteiro para o novo mapa, ou NULL se falhar na alocação
 * @note A capacidade inicial é INITIAL_CAPACITY
 */
struct IntegerMap* IntegerMap_new() {
    struct IntegerMap *map = malloc(sizeof(struct IntegerMap));
    if (map == NULL) return NULL;

    map->entries = malloc(sizeof(IntegerMapEntry) * INITIAL_CAPACITY);
    if (map->entries == NULL) {
        free(map);
        return NULL;
    }

    map->size = 0;
    map->capacity = INITIAL_CAPACITY;

    // Inicializar todas as entradas como inativas
    for (size_t i = 0; i < INITIAL_CAPACITY; i++) {
        map->entries[i].active = false;
    }

    return map;
}

/**
 * @brief Obtém o número de elementos ativos no mapa
 * @param map: Ponteiro para o mapa
 * @return Número de elementos ativos, ou 0 se map for NULL
 */
size_t IntegerMap_getSize(struct IntegerMap *map) {
    if (map == NULL) return 0;
    return map->size;
}

/**
 * @brief Aumenta a capacidade do mapa dinamicamente
 * @param map: Ponteiro para o mapa
 * @return true se o redimensionamento foi bem-sucedido, false caso contrário
 * @note A nova capacidade é aumentada em CAPACITY_INCREMENT
 */
static bool IntegerMap_resize(struct IntegerMap *map) {
    if (map == NULL) return false;

    size_t new_capacity = map->capacity + CAPACITY_INCREMENT;
    IntegerMapEntry *new_entries = realloc(map->entries, sizeof(IntegerMapEntry) * new_capacity);

    if (new_entries == NULL) return false;

    map->entries = new_entries;

    // Inicializar as novas entradas como inativas
    for (size_t i = map->capacity; i < new_capacity; i++) {
        map->entries[i].active = false;
    }

    map->capacity = new_capacity;
    return true;
}

/**
 * @brief Encontra o índice de uma chave no mapa (linear search)
 * @param map: Ponteiro para o mapa
 * @param key: Chave a buscar
 * @return Índice da chave se encontrada, ou -1 (size_t max) se não encontrada
 */
static size_t IntegerMap_findIndex(struct IntegerMap *map, uint32_t key) {
    if (map == NULL) return (size_t)-1;

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].active && map->entries[i].key == key) {
            return i;
        }
    }
    return (size_t)-1;
}

/**
 * @brief Define ou atualiza um par chave-valor no mapa
 * @param map: Ponteiro para o mapa
 * @param key: Chave do par
 * @param value: Valor do par
 * @note Se a chave já existe, apenas o valor é alterado
 * @note Se não há espaço, o mapa é redimensionado automaticamente
 */
void IntegerMap_set(struct IntegerMap *map, uint32_t key, uint32_t value) {
    if (map == NULL) return;

    // Buscar se a chave já existe
    size_t index = IntegerMap_findIndex(map, key);

    if (index != (size_t)-1) {
        // Chave já existe: apenas atualizar o valor
        map->entries[index].value = value;
        return;
    }

    // Chave não existe: adicionar novo par
    // Se está no limite de capacidade, redimensionar
    if (map->size >= map->capacity) {
        if (!IntegerMap_resize(map)) return;
    }

    // Encontrar primeira posição vazia
    for (size_t i = 0; i < map->capacity; i++) {
        if (!map->entries[i].active) {
            map->entries[i].key = key;
            map->entries[i].value = value;
            map->entries[i].active = true;
            map->size++;
            return;
        }
    }
}

/**
 * @brief Obtém o valor associado a uma chave
 * @param map: Ponteiro para o mapa
 * @param key: Chave a buscar
 * @param value: Ponteiro para receber o valor encontrado
 * @note Se a chave não for encontrada, value não é modificado
 */
void IntegerMap_get(struct IntegerMap *map, uint32_t key, uint32_t* value) {
    if (map == NULL || value == NULL) return;

    size_t index = IntegerMap_findIndex(map, key);
    if (index != (size_t)-1) {
        *value = map->entries[index].value;
    }
}

/**
 * @brief Remove uma chave do mapa
 * @param map: Ponteiro para o mapa
 * @param key: Chave a remover
 * @note A entrada é marcada como inativa, não é removida fisicamente (otimização)
 */
void IntegerMap_delete(struct IntegerMap *map, uint32_t key) {
    if (map == NULL) return;

    size_t index = IntegerMap_findIndex(map, key);
    if (index != (size_t)-1) {
        map->entries[index].active = false;
        map->size--;
    }
}

/**
 * @brief Libera toda a memória associada ao mapa
 * @param map: Ponteiro para o mapa (não é usado, deixado por compatibilidade com header)
 * @note O parâmetro key no header original não é necessário para liberar o mapa inteiro
 * @warning Após esta chamada, o ponteiro não deve ser mais utilizado
 */
void IntegerMap_free(struct IntegerMap *map, uint32_t key) {
    // Nota: O parâmetro 'key' não é necessário para liberar o mapa inteiro
    // Esta função libera toda a estrutura, não apenas uma entrada
    if (map == NULL) return;

    free(map->entries);
    free(map);
}
