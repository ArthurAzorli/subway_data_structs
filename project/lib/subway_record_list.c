#include "subway_record_list.h"
#include "../subway/subway_record_repository.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Nó da linked list contendo um registro de metrô
 * @note Cada nó armazena uma cópia do registro e um ponteiro para o próximo nó
 */
typedef struct SubwayRecordNode {
    struct SubwayRecord *record; // Ponteiro para o registro armazenado
    struct SubwayRecordNode *next; // Ponteiro para o próximo nó (NULL se último)
} SubwayRecordNode;

/**
 * @brief Estrutura da linked list de registros de metrô
 * @note Mantém referências ao primeiro e último nó para operações eficientes
 */
struct SubwayRecordList {
    SubwayRecordNode *head; // Primeiro nó da lista (NULL se vazia)
    SubwayRecordNode *tail; // Último nó da lista (NULL se vazia)
    size_t size; // Número de elementos na lista
};

/**
 * @brief Inicializa uma nova lista vazia de registros de metrô
 * @return Ponteiro para a nova lista, ou NULL se falhar na alocação
 */
struct SubwayRecordList *SubwayRecordList_init() {
    struct SubwayRecordList *list = malloc(sizeof(struct SubwayRecordList));
    if (list == NULL) return NULL;

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

/**
 * @brief Retorna o número de elementos armazenados na lista
 * @param list: Ponteiro para a lista
 * @return Número de elementos, ou 0 se list for NULL
 */
size_t SubwayRecordList_getSize(struct SubwayRecordList *list) {
    if (list == NULL) return 0;
    return list->size;
}

/**
 * @brief Obtém uma cópia do registro no índice especificado
 * @param list: Ponteiro para a lista
 * @param index: Índice do registro (0-based)
 * @param record: Ponteiro para struct SubwayRecord onde copiar os dados
 * @return true se conseguir obter o registro, false se índice inválido ou list/record for NULL
 * @note Copia os dados, não retorna uma referência ao nó interno
 */
bool SubwayRecordList_get(struct SubwayRecordList *list, size_t index, struct SubwayRecord *record) {
    if (list == NULL || record == NULL) return false;
    if (index >= list->size) return false;

    // Navegar até o nó no índice desejado
    SubwayRecordNode *current = list->head;
    for (size_t i = 0; i < index; i++) {
        if (current == NULL) return false;
        current = current->next;
    }

    if (current == NULL || current->record == NULL) return false;

    // Copiar todos os campos do registro original
    record->rrn = current->record->rrn;
    record->originStationID = current->record->originStationID;
    record->originLineID = current->record->originLineID;
    record->destinationStationID = current->record->destinationStationID;
    record->destinationDistant = current->record->destinationDistant;
    record->interactionStationID = current->record->interactionStationID;
    record->interactionLineID = current->record->interactionLineID;

    // Copiar stationName
    if (current->record->stationName != NULL) {
        record->stationName = strdup(current->record->stationName);
        if (record->stationName == NULL) {
            free(record);
            return false;
        }
        record->stationNameLength = current->record->stationNameLength;
    } else {
        record->stationName = NULL;
        record->stationNameLength = 0;
    }

    // Copiar lineName
    if (current->record->lineName != NULL) {
        record->lineName = strdup(current->record->lineName);
        if (record->lineName == NULL) {
            free(record->stationName);
            free(record);
            return false;
        }
        record->lineNameLength = current->record->lineNameLength;
    } else {
        record->lineName = NULL;
        record->lineNameLength = 0;
    }

    return true;
}

/**
 * @brief Adiciona um registro no final da lista
 * @param list: Ponteiro para a lista
 * @param record: Ponteiro para o registro a adicionar
 * @note O registro é copiado (deep copy dos dados), não armazenado por referência
 * @note Se o registro contiver strings, elas também são copiadas
 */
void SubwayRecordList_add(struct SubwayRecordList *list, struct SubwayRecord *record) {
    if (list == NULL || record == NULL) return;

    // Criar novo nó
    SubwayRecordNode *newNode = malloc(sizeof(SubwayRecordNode));
    if (newNode == NULL) return;

    // Alocar e copiar o registro
    struct SubwayRecord *copiedRecord = SubwayRecord_init();
    if (copiedRecord == NULL) {
        free(newNode);
        return;
    }

    // Copiar todos os campos do registro original
    copiedRecord->rrn = record->rrn;
    copiedRecord->originStationID = record->originStationID;
    copiedRecord->originLineID = record->originLineID;
    copiedRecord->destinationStationID = record->destinationStationID;
    copiedRecord->destinationDistant = record->destinationDistant;
    copiedRecord->interactionStationID = record->interactionStationID;
    copiedRecord->interactionLineID = record->interactionLineID;

    // Copiar stationName
    if (record->stationName != NULL) {
        copiedRecord->stationName = strdup(record->stationName);
        if (copiedRecord->stationName == NULL) {
            free(copiedRecord);
            free(newNode);
            return;
        }
        copiedRecord->stationNameLength = record->stationNameLength;
    } else {
        copiedRecord->stationName = NULL;
        copiedRecord->stationNameLength = 0;
    }

    // Copiar lineName
    if (record->lineName != NULL) {
        copiedRecord->lineName = strdup(record->lineName);
        if (copiedRecord->lineName == NULL) {
            free(copiedRecord->stationName);
            free(copiedRecord);
            free(newNode);
            return;
        }
        copiedRecord->lineNameLength = record->lineNameLength;
    } else {
        copiedRecord->lineName = NULL;
        copiedRecord->lineNameLength = 0;
    }

    newNode->record = copiedRecord;
    newNode->next = NULL;

    // Adicionar no final da lista
    if (list->head == NULL) {
        // Lista estava vazia
        list->head = newNode;
        list->tail = newNode;
    } else {
        // Adicionar após o tail
        list->tail->next = newNode;
        list->tail = newNode;
    }

    list->size++;
}

/**
 * @brief Remove o registro no índice especificado
 * @param list: Ponteiro para a lista
 * @param index: Índice do registro a remover (0-based)
 * @note Se o índice for inválido, nada acontece
 * @note A memória do nó e do registro é liberada
 */
void SubwayRecordList_remove(struct SubwayRecordList *list, size_t index) {
    if (list == NULL || index >= list->size) return;

    SubwayRecordNode *nodeToRemove;

    if (index == 0) {
        // Remover o primeiro nó
        nodeToRemove = list->head;
        list->head = list->head->next;

        if (list->head == NULL) {
            // Lista ficou vazia
            list->tail = NULL;
        }
    } else {
        // Encontrar o nó anterior ao que será removido
        SubwayRecordNode *previous = list->head;
        for (size_t i = 0; i < index - 1; i++) {
            if (previous == NULL) return;
            previous = previous->next;
        }

        nodeToRemove = previous->next;
        if (nodeToRemove == NULL) return;

        previous->next = nodeToRemove->next;

        // Atualizar tail se for removido o último nó
        if (nodeToRemove == list->tail) {
            list->tail = previous;
        }
    }

    // Liberar o registro dentro do nó
    if (nodeToRemove->record != NULL) {
        SubwayRecord_free(nodeToRemove->record);
    }

    // Liberar o nó
    free(nodeToRemove);
    list->size--;
}

void SubwayRecordList_removeByStationID(struct SubwayRecordList *list, uint32_t stationID) {
    if (list == NULL || list->size == 0) return;

    SubwayRecordNode *nodeToRemove;
    if (list->head->record->originStationID == stationID) {
        // Remover o primeiro nó
        nodeToRemove = list->head;
        list->head = list->head->next;

        if (list->head == NULL) {
            // Lista ficou vazia
            list->tail = NULL;
        }
    } else {
        // Encontrar o nó anterior ao que será removido
        SubwayRecordNode *previous = list->head;
        for (size_t i = 0; i < list->size; i++) {
            if (previous == NULL || previous->next == NULL) return;
            if (previous->next->record->originStationID == stationID) break;
            previous = previous->next;
        }

        nodeToRemove = previous->next;
        if (nodeToRemove == NULL) return;

        previous->next = nodeToRemove->next;

        // Atualizar tail se for removido o último nó
        if (nodeToRemove == list->tail) {
            list->tail = previous;
        }
    }

    // Liberar o registro dentro do nó
    if (nodeToRemove->record != NULL) {
        SubwayRecord_free(nodeToRemove->record);
    }

    // Liberar o nó
    free(nodeToRemove);
    list->size--;
}

/**
 * @brief Libera toda a memória associada à lista e seus registros
 * @param list: Ponteiro para a lista
 * @warning Após esta chamada, o ponteiro não deve ser mais utilizado
 */
void SubwayRecordList_free(struct SubwayRecordList *list) {
    if (list == NULL) return;

    SubwayRecordNode *current = list->head;
    while (current != NULL) {
        SubwayRecordNode *next = current->next;

        // Liberar o registro dentro do nó
        if (current->record != NULL) {
            SubwayRecord_free(current->record);
        }

        // Liberar o nó
        free(current);

        current = next;
    }

    // Liberar a estrutura da lista
    free(list);
}
