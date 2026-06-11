#ifndef INDEXABLE_RECORD_AVL_H
#define INDEXABLE_RECORD_AVL_H

#include "../indexable/indexable_record_repository.h"
#include <stddef.h>

struct IndexableRecordAVL{
    size_t height;
    size_t size;
    struct IndexableRecord *record;
    struct IndexableRecordAVL *left;
    struct IndexableRecordAVL *right;
};

struct IndexableRecordAVL *IndexableRecordAVL_push(struct IndexableRecordAVL *root, struct IndexableRecord *record);
struct IndexableRecordAVL *IndexableRecordAVL_remove(struct IndexableRecordAVL *root, uint32_t stationID);
struct IndexableRecord *IndexableRecordAVL_getByStationID(const struct IndexableRecordAVL *root, uint32_t stationID);
struct IndexableRecord *IndexableRecordAVL_getByIndex(const struct IndexableRecordAVL *root, size_t index);
void IndexableRecordAVL_free(struct IndexableRecordAVL *root);


#endif //INDEXABLE_RECORD_AVL_H
