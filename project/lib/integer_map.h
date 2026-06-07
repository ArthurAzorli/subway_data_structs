#ifndef INTEGER_MAP_H
#define INTEGER_MAP_H
#include <stdint.h>

struct IntegerMap;

struct IntegerMap* IntegerMap_new();
size_t IntegerMap_getSize(struct IntegerMap *map);
void IntegerMap_set(struct IntegerMap *map, uint32_t key, uint32_t value);
void IntegerMap_get(struct IntegerMap *map, uint32_t key, uint32_t* value);
void IntegerMap_delete(struct IntegerMap *map, uint32_t key);
void IntegerMap_free(struct IntegerMap *map, uint32_t key);

#endif //INTEGER_MAP_H
