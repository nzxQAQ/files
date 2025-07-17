#include "Graph.h"
#include "poodle.h"
#include <stdio.h>
#include <stdlib.h>

Edge *createEdge(int dest, int transmissionTime)
{
    Edge *newEdge = (Edge *)malloc(sizeof(Edge));
    if (newEdge)
    {
        newEdge->dest = dest;
        newEdge->transmissionTime = transmissionTime;
        newEdge->next = NULL;
    }
    return newEdge;
}

Graph *buildGraph(struct computer computers[], int numComputers,
                  struct connection connections[], int numConnections)
{
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (!graph)
        return NULL;

    graph->numComputers = numComputers;
    graph->array = (AdjList *)calloc(numComputers, sizeof(AdjList));
    if (!graph->array)
    {
        free(graph);
        return NULL;
    }

    for (int i = 0; i < numConnections; i++)
    {
        int src = connections[i].computerA;
        int dest = connections[i].computerB;
        int time = connections[i].transmissionTime;

        Edge *newEdge = createEdge(dest, time);
        if (newEdge)
        {
            newEdge->next = graph->array[src].head;
            graph->array[src].head = newEdge;
        }

        newEdge = createEdge(src, time);
        if (newEdge)
        {
            newEdge->next = graph->array[dest].head;
            graph->array[dest].head = newEdge;
        }
    }

    return graph;
}

void freeGraph(Graph *graph)
{
    if (graph)
    {
        if (graph->array)
        {
            for (int i = 0; i < graph->numComputers; i++)
            {
                Edge *current = graph->array[i].head;
                while (current)
                {
                    Edge *temp = current;
                    current = current->next;
                    free(temp);
                }
            }
            free(graph->array);
        }
        free(graph);
    }
}