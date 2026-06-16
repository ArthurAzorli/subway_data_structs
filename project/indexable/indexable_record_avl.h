#ifndef INDEXABLE_RECORD_AVL_H
#define INDEXABLE_RECORD_AVL_H

#include "indexable_record_repository.h"
#include <stddef.h>

/**
 * @struct IndexableRecordAVL
 * @brief Represents a node in an AVL tree for indexable subway records.
 *
 * This structure is used to build a balanced binary search tree (AVL) that
 * indexes subway records by their station ID. Each node contains:
 * - The height of the subtree (used for balancing).
 * - The size of the subtree (number of nodes).
 * - A pointer to the associated IndexableRecord.
 * - Pointers to the left and right child nodes.
 */
struct IndexableRecordAVL {
    size_t height;                  /**< Height of the subtree rooted at this node */
    size_t size;                    /**< Total number of nodes in the subtree */
    struct IndexableRecord *record; /**< Pointer to the indexable record stored in this node */
    struct IndexableRecordAVL *left; /**< Pointer to the left child node */
    struct IndexableRecordAVL *right; /**< Pointer to the right child node */
};


struct IndexableRecordAVL *IndexableRecordAVL_push(struct IndexableRecordAVL *root, struct IndexableRecord *record);
struct IndexableRecordAVL *IndexableRecordAVL_remove(struct IndexableRecordAVL *root, uint32_t stationID);
struct IndexableRecord *IndexableRecordAVL_getByStationID(const struct IndexableRecordAVL *root, uint32_t stationID);
struct IndexableRecord *IndexableRecordAVL_getByIndex(const struct IndexableRecordAVL *root, size_t index);
void IndexableRecordAVL_free(struct IndexableRecordAVL *root);

struct IndexableRecordAVL *IndexableRecordAVL_readFromFile(const char *fileName);
bool IndexableRecordAVL_writeOnFile(const char *fileName, struct IndexableRecordAVL *root);


#endif //INDEXABLE_RECORD_AVL_H
