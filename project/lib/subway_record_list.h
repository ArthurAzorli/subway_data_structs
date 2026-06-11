
#ifndef SUBWAY_RECORD_LIST_H
#define SUBWAY_RECORD_LIST_H
#include "../subway/subway_record_repository.h"

#include <stdbool.h>
#include <stdlib.h>


struct SubwayRecordList;

struct SubwayRecordList* SubwayRecordList_init();
size_t SubwayRecordList_getSize(struct SubwayRecordList*);
bool SubwayRecordList_get(struct SubwayRecordList*, size_t index, struct SubwayRecord*);
void SubwayRecordList_add(struct SubwayRecordList*, struct SubwayRecord*);
void SubwayRecordList_remove(struct SubwayRecordList*, size_t index);
void SubwayRecordList_free(struct SubwayRecordList*);

#endif //SUBWAY_RECORD_LIST_H
