//
// Created by User on 02/07/2026.
//

#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../services/types.h"

typedef struct GraphEdge {
    char *destinationStationName;
    uint32_t distance;
    char **lines;
    size_t linesCount;
    struct GraphEdge *next;
} GraphEdge;

typedef struct {
    char *stationName;
    GraphEdge *head;
} GraphVertex;

struct Graph {
    GraphVertex *vertices;
    size_t verticesCount;
    size_t capacity;
};

// ======= Private Graph Functions ======= \\

long Graph_findIndexByName(const struct Graph *graph, const char *name) {
    for (long i = 0; i < graph->verticesCount; i++) {
        if (strcmp(graph->vertices[i].stationName, name) == 0) return i;
    }
    return -1;
}

char *Graph_findNameByStationID(struct SubwayRecordList *list, uint32_t stationID) {
    if (list == NULL) return NULL;

    const size_t size = SubwayRecordList_getSize(list);
    for (size_t i = 0; i < size; i++) {
        struct SubwayRecord *record = SubwayRecord_init();
        if (!SubwayRecordList_get(list, i, record)) {
            SubwayRecord_free(record);
            return NULL;
        }

        if (record->originStationID == stationID && record->stationName != NULL) {
            char *stationName = strdup(record->stationName);
            SubwayRecord_free(record);
            return stationName;
        }

        SubwayRecord_free(record);
    }
    return NULL;
}

size_t Graph_getOrAddVertex(struct Graph *graph, const char *stationName) {
    //get index if exists station
    const long index = Graph_findIndexByName(graph, stationName);
    if (index != -1) return index;

    //realloc if it needs
    if (graph->verticesCount >= graph->capacity) {
        graph->capacity = graph->capacity += 10;
        graph->vertices = realloc(graph->vertices, graph->capacity * sizeof(GraphVertex));
    }

    //reorder station in alphabetic order
    size_t pos = graph->verticesCount;
    while (pos > 0 && strcmp(graph->vertices[pos - 1].stationName, stationName) > 0) {
        graph->vertices[pos] = graph->vertices[pos - 1];
        pos--;
    }

    // set new station vertex
    graph->vertices[pos].stationName = strdup(stationName);
    graph->vertices[pos].head = NULL;
    graph->verticesCount++;
    return pos;
}

void Graph_dfsPrintPrim(struct Graph *graph, const long index, bool visited[]) {
    visited[index] = true;
    GraphEdge *edge = graph->vertices[index].head;
    while (edge != NULL) {
        const long nextIndex = Graph_findIndexByName(graph, edge->destinationStationName);
        if (nextIndex != -1 && !visited[nextIndex]) {
            printf("%s, %s, %d\n", graph->vertices[index].stationName, edge->destinationStationName, edge->distance);
            Graph_dfsPrintPrim(graph, nextIndex, visited);
        }
        edge = edge->next;
    }
}

long Graph_dfsCyclesCount(struct Graph *graph, const long startIndex, const long currentIndex, bool visited[]) {
    visited[currentIndex] = true;
    long cyclesCount = 0;

    GraphEdge *edge = graph->vertices[currentIndex].head;
    while (edge != NULL) {
        const long nextIndex = Graph_findIndexByName(graph, edge->destinationStationName);
        if (nextIndex != -1) {
            if (nextIndex == startIndex) {
                cyclesCount++; //found a cycle to start
            } else if (!visited[nextIndex]) {
                // sum cycles count results
                cyclesCount += Graph_dfsCyclesCount(graph, startIndex, nextIndex, visited);
            }
        }
        edge = edge->next;
    }

    visited[currentIndex] = false; // Backtracking
    return cyclesCount;
}

// ======= Public Graph Functions ======= \\

struct Graph *Graph_init(const size_t capacity) {
    struct Graph *graph = malloc(sizeof(struct Graph));
    if (graph == NULL) return NULL;

    graph->capacity = capacity;
    graph->verticesCount = 0;

    graph->vertices = malloc(capacity * sizeof(GraphVertex));
    if (graph->vertices == NULL) {
        free(graph);
        return NULL;
    }

    return graph;
}


void Graph_fillByList(struct Graph *graph, struct SubwayRecordList *list) {
    if (graph == NULL || list == NULL) return;

    const size_t size = SubwayRecordList_getSize(list);
    for (size_t i = 0; i < size; i++) {
        struct SubwayRecord *record = SubwayRecord_init();
        if (!SubwayRecordList_get(list, i, record)) {
            SubwayRecord_free(record);
            return;
        }

        //origin to destination edge
        if (record->destinationStationID != EMPTY) {
            char *destinationName = Graph_findNameByStationID(list, record->destinationStationID);
            if (destinationName != NULL) {
                Graph_addEdge(graph, record->stationName, destinationName, record->destinationDistant,
                              record->lineName);
                free(destinationName);
            }
        }

        //origin to integration edge
        if (record->interactionStationID != EMPTY) {
            char *integrationName = Graph_findNameByStationID(list, record->interactionStationID);
            if (integrationName != NULL) {
                if (strcmp(record->stationName, integrationName) != 0) {
                    Graph_addEdge(graph, record->stationName, integrationName, 0, "Integração");
                }
                free(integrationName);
            }
        }

        SubwayRecord_free(record);
    }
}

void Graph_addEdge(struct Graph *graph, const char *origin, const char *destiny, const uint32_t distance,
                   const char *lineName) {
    Graph_getOrAddVertex(graph, destiny);
    const size_t originIndex = Graph_getOrAddVertex(graph, origin);


    GraphEdge *current = graph->vertices[originIndex].head;
    GraphEdge *previous = NULL;

    while (current != NULL && strcmp(current->destinationStationName, destiny) <= 0) {
        if (strcmp(current->destinationStationName, destiny) == 0) {
            //exit if line exists on the lines names list
            for (size_t i = 0; i < current->linesCount; i++) {
                if (strcmp(current->lines[i], lineName) == 0) return;
            }

            //realloc new space
            current->lines = realloc(current->lines, (current->linesCount + 1) * sizeof(char *));

            //insert in the order
            long pos = (long) current->linesCount;
            while (pos > 0 && strcmp(current->lines[pos - 1], lineName) > 0) {
                current->lines[pos] = current->lines[pos - 1];
                pos--;
            }

            current->lines[pos] = strdup(lineName);
            current->linesCount++;

            return;
        }
        previous = current;
        current = current->next;
    }

    //init a new edge
    GraphEdge *newNode = malloc(sizeof(GraphEdge));
    if (newNode == NULL) return;

    //set the params
    newNode->destinationStationName = strdup(destiny);
    newNode->distance = distance;
    newNode->next = current;
    newNode->lines = malloc(sizeof(char *));
    if (newNode->lines != NULL) {
        newNode->lines[0] = strdup(lineName);
        newNode->linesCount = 1;
    } else {
        newNode->linesCount = 0;
    }

    //update list links
    if (previous == NULL) {
        graph->vertices[originIndex].head = newNode;
    } else {
        previous->next = newNode;
    }
}

void Graph_print(const struct Graph *graph) {
    for (size_t i = 0; i < graph->verticesCount; i++) {
        const GraphVertex vertex = graph->vertices[i];
        if (vertex.head == NULL) continue;
        GraphEdge *edge = vertex.head;
        printf("%s", vertex.stationName);
        while (edge != NULL) {
            printf(", %s, %d", edge->destinationStationName, edge->distance);
            for (size_t j = 0; j < edge->linesCount; j++) {
                printf(", %s", edge->lines[j]);
            }

            edge = edge->next;
        }
        printf("\n");
    }
}

void Graph_dijstra(struct Graph *graph, const char *startStationName, const char *endStationName, size_t *pathLength,
                   char **path, uint32_t *distance) {
    if (graph == NULL || startStationName == NULL || endStationName == NULL) return;

    long originIndex = -1, destinyIndex = -1;
    for (long i = 0; i < graph->verticesCount; i++) {
        if (strcmp(graph->vertices[i].stationName, startStationName) == 0) originIndex = i;
        if (strcmp(graph->vertices[i].stationName, endStationName) == 0) destinyIndex = i;
    }

    *pathLength = 0;
    *distance = INFINITY;
    if (originIndex == -1 || destinyIndex == -1) return;

    uint32_t distances[graph->verticesCount];
    long parents[graph->verticesCount];
    bool visited[graph->verticesCount];

    for (size_t i = 0; i < graph->verticesCount; i++) {
        distances[i] = INFINITY;
        parents[i] = -1;
        visited[i] = false;
    }

    distances[originIndex] = 0;
    for (size_t i = 0; i < graph->verticesCount - 1; i++) {
        long u = -1;
        uint32_t minDistance = INFINITY;
        for (long j = 0; j < graph->verticesCount; j++) {
            if (!visited[j] && distances[j] < minDistance) {
                minDistance = distances[j];
                u = j;
            }
        }

        if (u == -1 || minDistance == INFINITY) break;
        visited[u] = true;

        GraphEdge *edge = graph->vertices[u].head;
        while (edge != NULL) {
            const long v = Graph_findIndexByName(graph, edge->destinationStationName);

            if (v != -1 && !visited[v]) {
                const uint32_t pathUDistance = distances[u] + edge->distance;
                if (pathUDistance < distances[v]) {
                    distances[v] = pathUDistance;
                    parents[v] = u;
                }
            }
            edge = edge->next;
        }
    }

    if (distances[destinyIndex] == INFINITY) return;

    *distance = distances[destinyIndex];
    long currentIndex = destinyIndex;
    while (currentIndex != -1 && *pathLength < graph->verticesCount) {
        path[*pathLength] = strdup(graph->vertices[currentIndex].stationName);
        currentIndex = parents[currentIndex];
        (*pathLength)++;
    }
}

bool Graph_printPrim(struct Graph *graph, const char *startStationName) {
    if (graph == NULL || startStationName == NULL || graph->verticesCount == 0) return false;

    struct Graph *undirGraph = Graph_init(graph->verticesCount);
    if (undirGraph == NULL) return false;

    for (size_t i = 0; i < graph->verticesCount; i++) {
        GraphEdge *edge = graph->vertices[i].head;
        while (edge != NULL) {
            Graph_addEdge(undirGraph, graph->vertices[i].stationName, edge->destinationStationName, edge->distance, "");
            Graph_addEdge(undirGraph, edge->destinationStationName, graph->vertices[i].stationName, edge->distance, "");
            edge = edge->next;
        }
    }

    const long startIndex = Graph_findIndexByName(undirGraph, startStationName);
    if (startIndex == -1) {
        Graph_free(undirGraph);
        return false;
    }

    struct Graph *minimumTree = Graph_init(graph->verticesCount);
    if (minimumTree == NULL) {
        Graph_free(undirGraph);
        return false;
    }

    bool visited[graph->verticesCount];
    long parents[graph->verticesCount];
    uint32_t distances[graph->verticesCount];

    for (size_t i = 0; i < graph->verticesCount; i++) {
        distances[i] = INFINITY;
        parents[i] = -1;
        visited[i] = false;
    }

    distances[startIndex] = 0;

    for (size_t i = 0; i < undirGraph->verticesCount; i++) {
        long u = -1;
        uint32_t dist = INFINITY;
        for (long j = 0; j < undirGraph->verticesCount; j++) {
            if (!visited[j] && distances[j] < dist) {
                dist = distances[j];
                u = j;
            }
        }

        if (u == -1 || dist == INFINITY) break;
        visited[u] = true;

        if (parents[u] != -1) {
            Graph_addEdge(minimumTree, undirGraph->vertices[parents[u]].stationName,
                          undirGraph->vertices[u].stationName, dist, "");
            Graph_addEdge(minimumTree, undirGraph->vertices[u].stationName,
                          undirGraph->vertices[parents[u]].stationName, dist, "");
        }

        GraphEdge *edge = undirGraph->vertices[u].head;
        while (edge != NULL) {
            const long v = Graph_findIndexByName(undirGraph, edge->destinationStationName);
            if (v != -1 && !visited[v]) {
                if (edge->distance < distances[v]) {
                    distances[v] = edge->distance;
                    parents[v] = u;
                } else if (edge->distance == distances[v] && parents[v] != -1 && u < parents[v]) {
                    parents[v] = u;
                }
            }
            edge = edge->next;
        }
    }

    const long treeStartIndex = Graph_findIndexByName(minimumTree, startStationName);
    if (treeStartIndex != -1) {
        bool visitedDFS[minimumTree->verticesCount];
        for (size_t i = 0; i < minimumTree->verticesCount; i++) {
            visitedDFS[i] = false;
        }
        Graph_dfsPrintPrim(minimumTree, treeStartIndex, visitedDFS);
    }

    Graph_free(undirGraph);
    Graph_free(minimumTree);
    return true;
}

long Graph_cyclesCount(struct Graph *graph, const char *startStationName) {
    if (graph == NULL || startStationName == NULL) return -1;
    const long currentIndex = Graph_findIndexByName(graph, startStationName);
    if (currentIndex == -1) return -1;

    bool visited[graph->verticesCount];
    for (size_t i = 0; i < graph->verticesCount; i++) {
        visited[i] = false;
    }

    return Graph_dfsCyclesCount(graph, currentIndex, currentIndex, visited);
}

char *Graph_getStationName(const struct Graph *graph, const size_t index) {
    if (graph == NULL || index >= graph->verticesCount) return NULL;
    return graph->vertices[index].stationName;
}

void Graph_free(struct Graph *graph) {
    if (graph == NULL) return;
    //free vertices
    for (int i = 0; i < graph->verticesCount; i++) {
        free(graph->vertices[i].stationName);

        //free edge list
        GraphEdge *current = graph->vertices[i].head;
        while (current != NULL) {
            GraphEdge *next = current->next;
            free(current->destinationStationName);
            for (size_t j = 0; j < current->linesCount; j++) {
                free(current->lines[j]);
            }
            free(current->lines);
            free(current);
            current = next;
        }
    }
    free(graph->vertices);
    free(graph);
}
