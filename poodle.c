
#include "Graph.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "poodle.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

// 创建computerList节点
static struct computerList *newComputerListNode(int computer)
{
	struct computerList *newNode = (struct computerList *)malloc(sizeof(struct computerList));
	if (newNode)
	{
		newNode->computer = computer;
		newNode->next = NULL;
	}
	return newNode;
}

// // 主函数实现
// struct poodleResult poodle(
// 	struct computer computers[], int numComputers,
// 	struct connection connections[], int numConnections,
// 	int startingComputer)
// {
// 	struct poodleResult result = {0, NULL};

// 	// 检查起始计算机是否有效
// 	if (startingComputer < 0 || startingComputer >= numComputers)
// 	{
// 		return result;
// 	}

// 	// 构建图
// 	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);
// 	if (!graph)
// 	{
// 		return result;
// 	}

// 	// BFS队列和相关数组
// 	int *queue = (int *)malloc(numComputers * sizeof(int));
// 	int *time = (int *)malloc(numComputers * sizeof(int));
// 	bool *visited = (bool *)calloc(numComputers, sizeof(bool));
// 	int *parent = (int *)malloc(numComputers * sizeof(int));

// 	// 初始化
// 	for (int i = 0; i < numComputers; i++)
// 	{
// 		time[i] = INT_MAX;
// 		parent[i] = -1;
// 	}

// 	int front = 0, rear = 0; // 队首指针与队尾指针
// 	queue[rear++] = startingComputer;

// 	time[startingComputer] = computers[startingComputer].poodleTime;
// 	parent[startingComputer] = -1;

// 	// BFS遍历
// 	while (front < rear)
// 	{
// 		int current = queue[front++];
// 		visited[current] = true;

// 		Edge *neighbor = graph->array[current].head;
// 		while (neighbor)
// 		{
// 			int neighborId = neighbor->dest;

// 			// 检查安全等级是否允许入侵
// 			if (!visited[neighborId] &&
// 				graph->computers[current].securityLevel + 1 >= graph->computers[neighborId].securityLevel)
// 			{
// 				parent[neighborId] = current;
// 				time[neighborId] = MIN(time[neighborId], time[current] + neighbor->transmissionTime + computers[neighborId].poodleTime);
// 				queue[rear++] = neighborId;
// 			}
// 			neighbor = neighbor->next;
// 		}
// 	}

// 	// 统计步骤数,并用来为result.steps分配内存
// 	result.numSteps = 0;
// 	for (int i = 0; i < numComputers; i++)
// 	{
// 		if (visited[i])
// 		{
// 			result.numSteps++;
// 		}
// 	}

// 	result.steps = (struct step *)calloc(result.numSteps, sizeof(struct step));

// 	// 填充步骤信息
// 	int stepIndex = 0;
// 	for (int i = 0; i < result.numSteps; i++)
// 	{
// 		int cur = queue[i];

// 		result.steps[stepIndex].computer = cur;
// 		result.steps[stepIndex].time = time[cur];

// 		// 构建recipients链表
// 		struct computerList *head = NULL;
// 		struct computerList **tail = &head;

// 		Edge *neighbor = graph->array[cur].head;
// 		while (neighbor)
// 		{
// 			int neighborId = neighbor->dest;

// 			// 检查是否是当前计算机入侵的
// 			if (parent[neighborId] == cur)
// 			{
// 				struct computerList *newNode = newComputerListNode(neighborId);
// 				if (newNode)
// 				{
// 					*tail = newNode;
// 					tail = &(newNode->next);
// 				}
// 			}
// 			neighbor = neighbor->next;
// 		}

// 		result.steps[stepIndex].recipients = head;
// 		stepIndex++;
// 	}

// 	// 释放资源
// 	free(queue);
// 	free(time);
// 	free(visited);
// 	free(parent);
// 	freeGraph(graph);

// 	return result;
// }

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
	bool *visited = (bool *)calloc(numComputers, sizeof(bool));
	bool *inResult = (bool *)calloc(numComputers, sizeof(bool));

	for (int i = 0; i < numComputers; i++)
	{
		time[i] = INT_MAX;
		parent[i] = -1;
	}

	time[startingComputer] = computers[startingComputer].poodleTime;
	parent[startingComputer] = -1;

	// Dijkstra算法
	for (int count = 0; count < numComputers; count++)
	{
		// 找到当前时间最短的未访问节点
		int u = -1;
		int minTime = INT_MAX;
		for (int v = 0; v < numComputers; v++)
		{
			if (!visited[v] && time[v] < minTime)
			{
				minTime = time[v];
				u = v;
			}
		}

		// 如果没有找到可访问的节点，提前结束
		if (u == -1)
			break;

		visited[u] = true;

		// 更新邻居节点的时间
		Edge *neighbor = graph->array[u].head;
		while (neighbor)
		{
			int v = neighbor->dest;
			int newTime = time[u] + neighbor->transmissionTime + computers[v].poodleTime;

			// 检查安全等级和更短时间
			if (!visited[v] &&
				computers[u].securityLevel + 1 >= computers[v].securityLevel &&
				newTime < time[v])
			{

				time[v] = newTime;
				parent[v] = u;
			}
			neighbor = neighbor->next;
		}
	}

	// 统计步骤数
	result.numSteps = 0;
	for (int i = 0; i < numComputers; i++)
	{
		if (visited[i])
		{
			result.numSteps++;
		}
	}

	// 创建步骤数组并按时间排序
	result.steps = (struct step *)calloc(result.numSteps, sizeof(struct step));
	int stepIndex = 0;
	for (int i = 0; i < numComputers; i++)
	{
		if (visited[i])
		{
			result.steps[stepIndex].computer = i;
			result.steps[stepIndex].time = time[i];
			stepIndex++;
		}
	}

	// 按时间排序步骤（简单的冒泡排序）
	for (int i = 0; i < result.numSteps - 1; i++)
	{
		for (int j = i + 1; j < result.numSteps; j++)
		{
			if (result.steps[i].time > result.steps[j].time)
			{
				struct step temp = result.steps[i];
				result.steps[i] = result.steps[j];
				result.steps[j] = temp;
			}
		}
	}

	// 填充recipients信息
	for (int i = 0; i < result.numSteps; i++)
	{
		int cur = result.steps[i].computer;
		struct computerList *head = NULL;
		struct computerList **tail = &head;

		// 找出所有子节点
		for (int j = 0; j < numComputers; j++)
		{
			if (parent[j] == cur)
			{
				struct computerList *newNode = newComputerListNode(j);
				if (newNode)
				{
					*tail = newNode;
					tail = &(newNode->next);
				}
			}
		}
		result.steps[i].recipients = head;
	}

	// 释放资源
	free(time);
	free(parent);
	free(visited);
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
