#include "subway_record_list.h"
#include "subway_record_repository.h"
#include <stdlib.h>
#include <string.h>

/**
 * @struct SubwayRecordNode
 * @brief Node of the linked list containing a subway record.
 *
 * Each node stores a deep copy of a SubwayRecord and a pointer to the next node.
 */
typedef struct SubwayRecordNode {
    struct SubwayRecord *record; /**< Pointer to the stored subway record */
    struct SubwayRecordNode *next; /**< Pointer to the next node (NULL if last) */
} SubwayRecordNode;

/**
 * @struct SubwayRecordList
 * @brief Linked list structure for subway records.
 *
 * Maintains references to the first and last node for efficient operations.
 */
struct SubwayRecordList {
    SubwayRecordNode *head; /**< First node of the list (NULL if empty) */
    SubwayRecordNode *tail; /**< Last node of the list (NULL if empty) */
    size_t size;            /**< Number of elements in the list */
};

/**
 * @brief Initializes a new empty subway record list.
 * @return Pointer to the new list, or NULL if allocation fails.
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
 * @brief Returns the number of elements stored in the list.
 * @param list Pointer to the list.
 * @return Number of elements, or 0 if list is NULL.
 */
size_t SubwayRecordList_getSize(struct SubwayRecordList *list) {
    if (list == NULL) return 0;
    return list->size;
}

/**
 * @brief Retrieves a copy of the record at the specified index.
 *
 * Copies all fields of the record, including strings (deep copy).
 *
 * @param list Pointer to the list.
 * @param index Index of the record (0-based).
 * @param record Pointer to a SubwayRecord where the data will be copied.
 * @return true if the record was successfully retrieved, false otherwise.
 *
 * @note Copies the data, does not return a reference to the internal node.
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
 * @brief Adds a record to the end of the list.
 *
 * Performs a deep copy of the record, including strings.
 *
 * @param list Pointer to the list.
 * @param record Pointer to the record to add.
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
 * @brief Removes the record at the specified index.
 *
 * Frees the memory associated with the node and its record.
 *
 * @param list Pointer to the list.
 * @param index Index of the record to remove (0-based).
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

/**
 * @brief Removes a record from the list by its station ID.
 *
 * Frees the memory associated with the node and its record.
 *
 * @param list Pointer to the list.
 * @param stationID ID of the station to remove.
 */
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
 * @brief Frees all memory associated with the list and its records.
 *
 * Recursively frees all nodes and their records.
 *
 * @param list Pointer to the list.
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
