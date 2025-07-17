
#include "Graph.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "poodle.h"

////////////////////////////////////////////////////////////////////////
// Task 1

/* 辅助函数：在邻接表中查找连接时间 */
static int findConnectionTime(Graph *graph, int src, int dest)
{
	if (src == dest)
		return 0; // 自环连接，可看作边长为0

	Edge *current = graph->array[src].head;
	while (current)
	{
		if (current->dest == dest)
		{
			return current->transmissionTime;
		}
		current = current->next;
	}
	return -1; // 未找到连接
}

struct probePathResult probePath(
	struct computer computers[], int numComputers,
	struct connection connections[], int numConnections,
	int path[], int pathLength)
{
	struct probePathResult res = {SUCCESS, 0};

	if (pathLength == 0)
	{
		return res;
	}

	// 构建邻接表图
	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);

	bool *visited = (bool *)calloc(graph->numComputers, sizeof(bool));

	for (int i = 0; i < numComputers; i++)
	{
		visited[i] = false;
	}

	if (pathLength == 0)
	{
		return res;
	}

	// 处理path[0]
	int countTime = 0;
	int prev = path[0];
	if (!visited[prev])
	{
		countTime += computers[prev].poodleTime;
		visited[prev] = true;
	}

	// 处理path[1]到path[pathLength-1]
	for (int i = 1; i < pathLength; i++)
	{
		int current = path[i];

		// 检查连接是否存在，若连接不存在，则res.status转为NO_CONNECTION
		int transmissionTime = findConnectionTime(graph, prev, current);
		if (transmissionTime == -1)
		{
			res.status = NO_CONNECTION;
			res.elapsedTime = countTime;
			free(visited);
			freeGraph(graph);
			return res;
		}

		// 检查安全等级是否合法，若安全权限不足，则res.status转为NO_PERMISSION
		if (computers[prev].securityLevel + 1 < computers[current].securityLevel)
		{
			res.status = NO_PERMISSION;
			res.elapsedTime = countTime;
			free(visited);
			freeGraph(graph);
			return res;
		}

		// 累加传输时间transmissionTime(边的权重)
		countTime += transmissionTime;

		// 只有初次访问该计算机，才需要计算poodleTime(点的权重)
		if (!visited[current])
		{
			countTime += computers[current].poodleTime;
			visited[current] = true;
		}

		prev = current;
	}

	res.elapsedTime = countTime;
	free(visited);
	freeGraph(graph);
	return res;
}

////////////////////////////////////////////////////////////////////////
// Task 2

// DFS函数,遍历节点v可入侵的邻居节点
static void dfs(Graph *graph, int v, bool visited[], int *count)
{
	visited[v] = true;
	(*count)++;

	Edge *node = graph->array[v].head;
	while (node)
	{
		int neighbor = node->dest;
		// 检查安全等级是否允许入侵
		if (!visited[neighbor] &&
			graph->computers[v].securityLevel + 1 >= graph->computers[neighbor].securityLevel)
		{
			dfs(graph, neighbor, visited, count);
		}
		node = node->next;
	}
}

struct chooseSourceResult chooseSource(
	struct computer computers[], int numComputers,
	struct connection connections[], int numConnections)
{
	struct chooseSourceResult res = {0, 0, NULL};

	// 构建图
	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);
	if (!graph)
		return res;

	int maxCount = 0;
	int bestSource = 0;
	int *bestComputers = NULL;

	// 遍历所有计算机作为源节点
	for (int src = 0; src < numComputers; src++)
	{
		bool *visited = (bool *)calloc(numComputers, sizeof(bool));

		int count = 0;
		dfs(graph, src, visited, &count);

		// 更新最大计数和最佳源节点
		if (count > maxCount)
		{
			maxCount = count;
			bestSource = src;

			// 更新被入侵的计算机列表
			free(bestComputers);
			bestComputers = (int *)malloc(maxCount * sizeof(int));

			int index = 0;
			for (int i = 0; i < numComputers; i++)
			{
				if (visited[i])
				{
					bestComputers[index++] = i;
				}
			}
		}

		free(visited);
	}

	freeGraph(graph);

	// 设置结果
	res.sourceComputer = bestSource;
	res.numComputers = maxCount;
	res.computers = bestComputers;

	return res;
}

////////////////////////////////////////////////////////////////////////
// Task 3

struct poodleResult poodle(
	struct computer computers[], int numComputers,
	struct connection connections[], int numConnections,
	int sourceComputer)
{
	struct poodleResult res = {0, NULL};

	return res;
}

////////////////////////////////////////////////////////////////////////
// Task 4

/**
 * Describe your solution in detail here:
 *
 * TODO
 */
struct poodleResult advancedPoodle(
	struct computer computers[], int numComputers,
	struct connection connections[], int numConnections,
	int sourceComputer)
{
	struct poodleResult res = {0, NULL};

	return res;
}
