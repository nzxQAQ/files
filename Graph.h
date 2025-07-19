#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include "poodle.h"

// 边链表
typedef struct Edge
{
    int dest;             // 这条边所连接的点的索引
    int transmissionTime; // 传输时间(即边的权重)
    struct Edge *next;    // 指向下一条边的指针
} Edge;

// 邻接表(一个存储节点信息和链表的数组)
typedef struct AdjList
{
    struct computer computer; // 存储该节点的计算机信息
    Edge *headEdge;           // 指向边链表的指针
} AdjList;

// 图
typedef struct Graph
{
    int numComputers;
    AdjList *array; // 数组
    struct computer *computers;
} Graph;

// 创建边
Edge *createEdge(int dest, int transmissionTime);

// 构建邻接表图
Graph *buildGraph(struct computer computers[], int numComputers, struct connection connections[], int numConnections);

// 释放图的内存
void freeGraph(Graph *graph);

#endif // GRAPH_H