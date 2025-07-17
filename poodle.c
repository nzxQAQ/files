
#include "Graph.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "poodle.h"
#define MAX_NUM 100 // 题目约束的计算机的最大数量

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

// 对数组的前n个元素排序
void bubbleSort(int arr[], int n)
{
	bool swapped;
	for (int i = 0; i < n - 1; i++)
	{
		swapped = false;
		for (int j = 0; j < n - i - 1; j++)
		{
			if (arr[j] > arr[j + 1])
			{
				int temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
				swapped = true;
			}
		}
		if (!swapped)
		{
			break;
		}
	}
}

struct poodleResult poodle(
	struct computer computers[], int numComputers,
	struct connection connections[], int numConnections,
	int startingComputer)
{
	struct poodleResult result = {0, NULL};

	// 检查起始计算机是否有效
	if (startingComputer < 0 || startingComputer >= numComputers)
	{
		return result;
	}

	// 构建图
	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);
	if (!graph)
	{
		return result;
	}

	// 初始化
	int *time = (int *)malloc(numComputers * sizeof(int));
	int *parent = (int *)malloc(numComputers * sizeof(int));
	int *queue = (int *)malloc(MAX_NUM * sizeof(int));
	bool *inResult = (bool *)calloc(numComputers, sizeof(bool));

	for (int i = 0; i < numComputers; i++)
	{
		time[i] = INT_MAX;
		parent[i] = -1;
		queue[i] = -1;
	}

	time[startingComputer] = computers[startingComputer].poodleTime;
	parent[startingComputer] = -1;

	int stepcount = 0;
	// Dijkstra算法,最多尝试更新numComputers次。如果无法找到符合要求的节点，则会提前结束。
	for (int i = 0; i < numComputers; i++)
	{
		// 找到当前时间最短且未访问的节点u
		int u = -1;
		int minTime = INT_MAX;
		for (int v = 0; v < numComputers; v++)
		{
			if (!inResult[v] && time[v] < minTime)
			{
				minTime = time[v];
				u = v;
			}
		}

		// 如果无法找到符合要求的节点，提前结束
		if (u == -1)
			break;

		// 如果找到了符合要求的节点，则将其标记为inResult，并将其加入queue的队尾
		inResult[u] = true;
		queue[stepcount++] = u;

		// 尝试更新邻居节点的时间
		Edge *neighbor = graph->array[u].head;
		while (neighbor)
		{
			int v = neighbor->dest;
			int newTime = time[u] + neighbor->transmissionTime + computers[v].poodleTime;

			// 如果v未被标记为inResult，且安全等级合法，并且时间可以变得更短，则更新时间
			if (!inResult[v] &&
				computers[u].securityLevel + 1 >= computers[v].securityLevel &&
				newTime < time[v])
			{
				time[v] = newTime;
				parent[v] = u;
			}
			neighbor = neighbor->next;
		}
	}

	// 算法结束后，将代表步骤数的stepcount赋值给result (即所有可以被入侵的计算机数量)
	result.numSteps = stepcount;

	// 初始化步骤数组result.steps，并按queue的顺序，对每一步填充计算机序号cur，以及它被入侵的时刻
	result.steps = (struct step *)calloc(result.numSteps, sizeof(struct step));

	for (int i = 0; i < stepcount; i++)
	{
		int cur = queue[i];
		result.steps[i].computer = cur;
		result.steps[i].time = time[cur];

		struct computerList *recipients = NULL;
		struct computerList **tail = &recipients;

		// 找出cur入侵的所有子节点,并且确保按升序输出
		int *sortArray = (int *)malloc(MAX_NUM * sizeof(int));
		int sortArrayIndex = 0;
		Edge *neighbor = graph->array[cur].head;
		while (neighbor)
		{
			int d = neighbor->dest;
			if (parent[d] == cur)
			{
				sortArray[sortArrayIndex++] = d;
			}
			neighbor = neighbor->next;
		}

		bubbleSort(sortArray, sortArrayIndex);

		for (int i = 0; i < sortArrayIndex; i++)
		{
			// 为排序后的sortArray中的元素，创建computerList节点，并插入链表
			struct computerList *newNode = (struct computerList *)malloc(sizeof(struct computerList));
			newNode->computer = sortArray[i];
			newNode->next = NULL;

			*tail = newNode;
			tail = &(newNode->next);
		}

		result.steps[i].recipients = recipients;
		free(sortArray);
	}

	// 释放内存资源
	free(time);
	free(parent);
	free(queue);
	free(inResult);
	freeGraph(graph);

	return result;
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
