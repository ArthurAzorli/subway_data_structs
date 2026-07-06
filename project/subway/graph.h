
#ifndef TRABALHO01_GRAPH_H
#define TRABALHO01_GRAPH_H
#include "subway_record_list.h"
#include <stdint.h>

struct Graph;

struct Graph *Graph_init(size_t capacity);
void Graph_fillByList(struct Graph *graph, struct SubwayRecordList *list);
void Graph_addEdge(struct Graph *graph, const char *origin, const char *destiny, uint32_t distance, const char *lineName);
void Graph_print(const struct Graph *graph);
void Graph_dijstra(struct Graph *graph, const char *startStationName, const char *endStationName, size_t *pathLength, char **path, uint32_t*distance);
bool Graph_printPrim(struct Graph *graph, const char *startStationName);
long Graph_cyclesCount(struct Graph *graph, const char* startStationName);
char* Graph_getStationName(const struct Graph *graph, size_t index);
void Graph_free(struct Graph *graph);



#endif //TRABALHO01_GRAPH_H