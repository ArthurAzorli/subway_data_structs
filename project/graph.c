#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "subway/subway_record_list.h"
#include "subway/subway_record_repository.h"

// ============================================================================
// 1. ESTRUTURAS OFICIAIS DO GRAFO (EXIGÊNCIA OBRIGATÓRIA DO ENUNCIADO)
// ============================================================================

typedef struct GraphEdge {
    char *nextStation;         // Nome da próxima estação
    int distance;              // Distância para a próxima estação
    char *lineName;            // Nome da linha associada ou "Integração"
    struct GraphEdge *next;    // Ponteiro para a próxima aresta da lista linear
} GraphEdge;

typedef struct GraphVertex {
    char *stationName;         // Nome do vértice (Estação)
    GraphEdge *head;           // Início da lista linear de adjacências
} GraphVertex;

typedef struct Graph {
    GraphVertex *vertices;     // Vetor de V elementos
    int numVertices;           // Quantidade atual de vértices únicos
    int capacity;              // Capacidade alocada para o vetor principal
} Graph;

// ============================================================================
// 2. FUNÇÕES AUXILIARES DE MANIPULAÇÃO E BUSCA ENCAPSULADA
// ============================================================================

// Traduz um ID de Estação em String fazendo buscas estritamente pelas funções públicas da lista
char *buscarNomePorID(struct SubwayRecordList *list, uint32_t stationID) {
    if (list == NULL || stationID == (uint32_t)-1) return NULL;

    size_t size = SubwayRecordList_getSize(list);
    for (size_t i = 0; i < size; i++) {
        struct SubwayRecord rec;
        if (SubwayRecordList_get(list, i, &rec)) {
            if (rec.originStationID == stationID && rec.stationName != NULL) {
                char *nomeEncontrado = strdup(rec.stationName);
                free(rec.stationName);
                free(rec.lineName);
                return nomeEncontrado; // Retorna cópia alocada (deve ser limpa depois)
            }
            free(rec.stationName);
            free(rec.lineName);
        }
    }
    return NULL;
}

// Busca ou insere um vértice mantendo o vetor principal ordenado alfabeticamente
int Graph_getOrAddVertex(Graph *g, const char *name) {
    for (int i = 0; i < g->numVertices; i++) {
        if (strcmp(g->vertices[i].stationName, name) == 0) {
            return i;
        }
    }

    if (g->numVertices >= g->capacity) {
        g->capacity = g->capacity == 0 ? 10 : g->capacity * 2;
        g->vertices = realloc(g->vertices, g->capacity * sizeof(GraphVertex));
    }

    // Insertion Sort para garantir a ordenação alfabética crescente do vetor de vértices V
    int pos = g->numVertices;
    while (pos > 0 && strcmp(g->vertices[pos - 1].stationName, name) > 0) {
        g->vertices[pos] = g->vertices[pos - 1];
        pos--;
    }

    g->vertices[pos].stationName = strdup(name);
    g->vertices[pos].head = NULL;
    g->numVertices++;
    return pos;
}

// Insere uma aresta na lista linear de forma ordenada (Estação crescente -> Linha crescente)
void Graph_addEdge(Graph *g, const char *src, const char *dest, int dist, const char *line) {
    int srcIdx = Graph_getOrAddVertex(g, src);
    Graph_getOrAddVertex(g, dest); // Garante a existência do destino no vetor principal

    GraphEdge *newNode = malloc(sizeof(GraphEdge));
    newNode->nextStation = strdup(dest);
    newNode->distance = dist;
    newNode->lineName = strdup(line ? line : "");
    newNode->next = NULL;

    GraphEdge *curr = g->vertices[srcIdx].head;
    GraphEdge *prev = NULL;

    while (curr != NULL) {
        int cmpName = strcmp(curr->nextStation, dest);
        if (cmpName > 0 || (cmpName == 0 && strcmp(curr->lineName, line) > 0)) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) {
        newNode->next = g->vertices[srcIdx].head;
        g->vertices[srcIdx].head = newNode;
    } else {
        newNode->next = curr;
        prev->next = newNode;
    }
}

void Graph_free(Graph *g) {
    if (g == NULL) return;
    for (int i = 0; i < g->numVertices; i++) {
        free(g->vertices[i].stationName);
        GraphEdge *curr = g->vertices[i].head;
        while (curr != NULL) {
            GraphEdge *next = curr->next;
            free(curr->nextStation);
            free(curr->lineName);
            free(curr);
            curr = next;
        }
    }
    free(g->vertices);
}

// Constrói o Grafo respeitando o encapsulamento total da SubwayRecordList
Graph *gerarGrafoDoRepositorio(struct SubwayRecordList *list) {
    if (list == NULL) return NULL;

    Graph *g = malloc(sizeof(Graph));
    g->vertices = NULL; g->numVertices = 0; g->capacity = 0;

    size_t size = SubwayRecordList_getSize(list);
    for (size_t i = 0; i < size; i++) {
        struct SubwayRecord record;
        if (SubwayRecordList_get(list, i, &record)) {

            // Requisito 1: Aresta para a próxima estação
            if (record.stationName != NULL && record.destinationStationID != (uint32_t)-1) {
                char *nomeProx = buscarNomePorID(list, record.destinationStationID);
                if (nomeProx != NULL) {
                    Graph_addEdge(g, record.stationName, nomeProx, record.destinationDistant, record.lineName);
                    free(nomeProx);
                }
            }

            // Requisito 2: Aresta de Integração
            if (record.stationName != NULL && record.interactionStationID != (uint32_t)-1) {
                char *nomeIntegra = buscarNomePorID(list, record.interactionStationID);
                if (nomeIntegra != NULL && strcmp(record.stationName, nomeIntegra) != 0) {
                    Graph_addEdge(g, record.stationName, nomeIntegra, 0, "Integração");
                    free(nomeIntegra);
                }
            }

            // Libera as strings alocadas pela cópia profunda do SubwayRecordList_get
            free(record.stationName);
            free(record.lineName);
        }
    }
    return g;
}

// ============================================================================
// 3. FUNCIONALIDADE [10]: EXIBIÇÃO DAS LISTAS DE ADJACÊNCIAS
// ============================================================================
void funcionalidade10(struct SubwayRecordList *list) {
    Graph *g = gerarGrafoDoRepositorio(list);
    if (g == NULL || g->numVertices == 0) {
        printf("Falha na execução da funcionalidade.\n");
        if (g) free(g);
        return;
    }

    for (int i = 0; i < g->numVertices; i++) {
        GraphEdge *curr = g->vertices[i].head;
        if (curr != NULL) {
            printf("%s", g->vertices[i].stationName);
            while (curr != NULL) {
                printf(", %s, %d, %s", curr->nextStation, curr->distance, curr->lineName);
                curr = curr->next;
            }
            printf("\n");
        }
    }
    Graph_free(g);
    free(g);
}

// ============================================================================
// 4. FUNCIONALIDADE [11]: DIJKSTRA (CAMINHO MAIS CURTO)
// ============================================================================
void funcionalidade11(struct SubwayRecordList *list, const char *origem, const char *destino) {
    Graph *g = gerarGrafoDoRepositorio(list);
    if (g == NULL) { printf("Falha na execução da funcionalidade.\n"); return; }

    int srcIdx = -1, destIdx = -1;
    for (int i = 0; i < g->numVertices; i++) {
        if (strcmp(g->vertices[i].stationName, origem) == 0) srcIdx = i;
        if (strcmp(g->vertices[i].stationName, destino) == 0) destIdx = i;
    }

    if (srcIdx == -1 || destIdx == -1) {
        printf("Não existe caminho entre as estações solicitadas.\n");
        Graph_free(g); free(g); return;
    }

    int *dist = malloc(g->numVertices * sizeof(int));
    int *parent = malloc(g->numVertices * sizeof(int));
    bool *visited = calloc(g->numVertices, sizeof(bool));

    for (int i = 0; i < g->numVertices; i++) {
        dist[i] = 1e9; parent[i] = -1;
    }
    dist[srcIdx] = 0;

    for (int count = 0; count < g->numVertices - 1; count++) {
        int u = -1; int minDist = 1e9;

        // Desempate: Se houver distâncias iguais, seleciona o menor índice (que corresponde à ordem alfabética)
        for (int v = 0; v < g->numVertices; v++) {
            if (!visited[v] && dist[v] < minDist) {
                minDist = dist[v]; u = v;
            }
        }

        if (u == -1 || dist[u] == 1e9) break;
        visited[u] = true;

        GraphEdge *edge = g->vertices[u].head;
        while (edge != NULL) {
            int v = -1;
            for (int i = 0; i < g->numVertices; i++) {
                if (strcmp(g->vertices[i].stationName, edge->nextStation) == 0) { v = i; break; }
            }

            if (v != -1 && !visited[v]) {
                if (dist[u] + edge->distance < dist[v]) {
                    dist[v] = dist[u] + edge->distance;
                    parent[v] = u;
                }
                // Regra de desempate para pesos de caminhos idênticos
                else if (dist[u] + edge->distance == dist[v]) {
                    int currParent = parent[v];
                    if (u < currParent) {
                        parent[v] = u;
                    }
                }
            }
            edge = edge->next;
        }
    }

    if (dist[destIdx] == 1e9) {
        printf("Não existe caminho entre as estações solicitadas.\n");
    } else {
        int path[200], pathLen = 0;
        int currPath = destIdx;
        while (currPath != -1) {
            path[pathLen++] = currPath;
            currPath = parent[currPath];
        }

        printf("Numero de estacoes que serao percorridas: %d\n", pathLen - 1);
        printf("Distancia que sera percorrida: %d\n", dist[destIdx]);

        for (int i = pathLen - 1; i >= 0; i--) {
            printf("%s%s", g->vertices[path[i]].stationName, i == 0 ? "" : ", ");
        }
        printf("\n");
    }

    free(dist); free(parent); free(visited);
    Graph_free(g); free(g);
}

// ============================================================================
// 5. FUNCIONALIDADE [12]: ÁRVORE GERADORA MÍNIMA (PRIM) + DFS
// ============================================================================
void dfsMST(Graph *mst, int u, bool *vis) {
    vis[u] = true;
    GraphEdge *edge = mst->vertices[u].head;
    while (edge != NULL) {
        int v = -1;
        for (int i = 0; i < mst->numVertices; i++) {
            if (strcmp(mst->vertices[i].stationName, edge->nextStation) == 0) { v = i; break; }
        }
        if (v != -1 && !vis[v]) {
            printf("%s, %s, %d\n", mst->vertices[u].stationName, edge->nextStation, edge->distance);
            dfsMST(mst, v, vis);
        }
        edge = edge->next;
    }
}

void funcionalidade12(struct SubwayRecordList *list, const char *startStation) {
    Graph *g = gerarGrafoDoRepositorio(list);
    if (g == NULL || g->numVertices == 0) { printf("Falha na execução da funcionalidade.\n"); return; }

    // Cria um grafo temporário Não-Direcionado (inserindo arestas bidirecionais de ida e volta)
    Graph undirG = {NULL, 0, 0};
    for (int i = 0; i < g->numVertices; i++) {
        GraphEdge *e = g->vertices[i].head;
        while (e != NULL) {
            Graph_addEdge(&undirG, g->vertices[i].stationName, e->nextStation, e->distance, e->lineName);
            Graph_addEdge(&undirG, e->nextStation, g->vertices[i].stationName, e->distance, e->lineName);
            e = e->next;
        }
    }

    int startIdx = -1;
    for (int i = 0; i < undirG.numVertices; i++) {
        if (strcmp(undirG.vertices[i].stationName, startStation) == 0) { startIdx = i; break; }
    }
    if (startIdx == -1) {
        printf("Falha na execução da funcionalidade.\n");
        Graph_free(&undirG); Graph_free(g); free(g); return;
    }

    Graph mst = {NULL, 0, 0};
    bool *inMST = calloc(undirG.numVertices, sizeof(bool));
    int *minEdge = malloc(undirG.numVertices * sizeof(int));
    int *parent = malloc(undirG.numVertices * sizeof(int));

    for (int i = 0; i < undirG.numVertices; i++) { minEdge[i] = 1e9; parent[i] = -1; }
    minEdge[startIdx] = 0;

    for (int step = 0; step < undirG.numVertices; step++) {
        int u = -1; int minDist = 1e9;
        for (int i = 0; i < undirG.numVertices; i++) {
            if (!inMST[i] && minEdge[i] < minDist) { minDist = minEdge[i]; u = i; }
        }

        if (u == -1 || minDist == 1e9) break;
        inMST[u] = true;

        if (parent[u] != -1) {
            Graph_addEdge(&mst, undirG.vertices[parent[u]].stationName, undirG.vertices[u].stationName, minDist, "");
            Graph_addEdge(&mst, undirG.vertices[u].stationName, undirG.vertices[parent[u]].stationName, minDist, "");
        }

        GraphEdge *e = undirG.vertices[u].head;
        while (e != NULL) {
            int v = -1;
            for (int i = 0; i < undirG.numVertices; i++) {
                if (strcmp(undirG.vertices[i].stationName, e->nextStation) == 0) { v = i; break; }
            }
            if (v != -1 && !inMST[v] && e->distance < minEdge[v]) {
                minEdge[v] = e->distance; parent[v] = u;
            }
            else if (v != -1 && !inMST[v] && e->distance == minEdge[v] && parent[v] != -1) {
                if (u < parent[v]) { parent[v] = u; }
            }
            e = e->next;
        }
    }

    int mstStartIdx = -1;
    for (int i = 0; i < mst.numVertices; i++) {
        if (strcmp(mst.vertices[i].stationName, startStation) == 0) { mstStartIdx = i; break; }
    }

    if (mstStartIdx != -1) {
        bool *visitedDFS = calloc(mst.numVertices, sizeof(bool));
        dfsMST(&mst, mstStartIdx, visitedDFS);
        free(visitedDFS);
    }

    free(inMST); free(minEdge); free(parent);
    Graph_free(&undirG); Graph_free(&mst); Graph_free(g); free(g);
}

// ============================================================================
// 6. FUNCIONALIDADE [13]: CONTAGEM DE CICLOS SIMPLES
// ============================================================================
void dfsContarCiclos(Graph *g, int currIdx, int startIdx, bool *visited, int *total) {
    visited[currIdx] = true;
    GraphEdge *edge = g->vertices[currIdx].head;
    while (edge != NULL) {
        int nextIdx = -1;
        for (int i = 0; i < g->numVertices; i++) {
            if (strcmp(g->vertices[i].stationName, edge->nextStation) == 0) { nextIdx = i; break; }
        }

        if (nextIdx != -1) {
            if (nextIdx == startIdx) {
                (*total)++;
            } else if (!visited[nextIdx]) {
                dfsContarCiclos(g, nextIdx, startIdx, visited, total);
            }
        }
        edge = edge->next;
    }
    visited[currIdx] = false; // Backtracking legítimo
}

void funcionalidade13(struct SubwayRecordList *list, const char *origem) {
    Graph *g = gerarGrafoDoRepositorio(list);
    int startIdx = -1;
    for (int i = 0; i < g->numVertices; i++) {
        if (strcmp(g->vertices[i].stationName, origem) == 0) { startIdx = i; break; }
    }

    if (startIdx == -1) {
        printf("Quantidade de ciclos: -1\n");
        if (g) { Graph_free(g); free(g); }
        return;
    }

    bool *visited = calloc(g->numVertices, sizeof(bool));
    int totalCycles = 0;

    dfsContarCiclos(g, startIdx, startIdx, visited, &totalCycles);

    if (totalCycles == 0) {
        printf("Quantidade de ciclos: -1\n");
    } else {
        printf("Quantidade de ciclos: %d\n", totalCycles);
    }
    
    free(visited);
    Graph_free(g);
    free(g);
}