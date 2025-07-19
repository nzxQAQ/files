
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

	Edge *edge = graph->array[src].headEdge;
	while (edge)
	{
		if (edge->dest == dest)
		{
			return edge->transmissionTime;
		}
		edge = edge->next;
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

// DFS函数,遍历节点u可入侵的邻居节点
static void dfs(Graph *graph, int u, bool visited[], int *count)
{
	visited[u] = true;
	(*count)++;

	Edge *edge = graph->array[u].headEdge;
	while (edge)
	{
		// v 是 u 的邻居节点
		int v = edge->dest;

		// 检查安全等级是否允许 u 入侵 v
		if (!visited[v] &&
			graph->computers[u].securityLevel + 1 >= graph->computers[v].securityLevel)
		{
			dfs(graph, v, visited, count);
		}
		edge = edge->next;
	}
}

struct chooseSourceResult chooseSource(
	struct computer computers[], int numComputers,
	struct connection connections[], int numConnections)
{
	struct chooseSourceResult res = {0, 0, NULL};

	// 构建图
	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);

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
	struct poodleResult res = {0, NULL};

	// 构建图
	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);

	// 初始化
	int *time = (int *)malloc(numComputers * sizeof(int));
	int *parent = (int *)malloc(numComputers * sizeof(int));
	int *resQueue = (int *)malloc(MAX_NUM * sizeof(int));
	bool *inDjikstra = (bool *)calloc(numComputers, sizeof(bool));

	for (int i = 0; i < numComputers; i++)
	{
		time[i] = INT_MAX;
		parent[i] = -1;
		resQueue[i] = -1;
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
			if (!inDjikstra[v] && time[v] < minTime)
			{
				minTime = time[v];
				u = v;
			}
		}

		// 如果无法找到符合要求的节点，提前结束
		if (u == -1)
			break;

		// 如果找到了符合要求的节点，则将其标记为inResult，并将其加入queue的队尾
		inDjikstra[u] = true;
		resQueue[stepcount++] = u;

		// 尝试更新邻居节点的时间
		Edge *edge = graph->array[u].headEdge;
		while (edge)
		{
			int v = edge->dest;
			int newTime = time[u] + edge->transmissionTime + computers[v].poodleTime;

			// 如果v未被标记为inResult，且安全等级合法，并且时间可以变得更短，则更新时间
			if (!inDjikstra[v] &&
				computers[u].securityLevel + 1 >= computers[v].securityLevel &&
				newTime < time[v])
			{
				time[v] = newTime;
				parent[v] = u;
			}
			edge = edge->next;
		}
	}

	// 算法结束后，将代表步骤数的stepcount赋值给result (即所有可以被入侵的计算机数量)
	res.numSteps = stepcount;

	// 初始化步骤数组result.steps，并按queue的顺序，对每一步填充计算机序号cur，以及它被入侵的时刻
	res.steps = (struct step *)calloc(res.numSteps, sizeof(struct step));

	for (int i = 0; i < stepcount; i++)
	{
		int cur = resQueue[i];
		res.steps[i].computer = cur;
		res.steps[i].time = time[cur];

		struct computerList *recipients = NULL;
		struct computerList **tail = &recipients;

		// task3的专属任务：找出cur入侵的所有子节点,并且确保按升序输出
		int *sortArray = (int *)malloc(MAX_NUM * sizeof(int));
		int sortArrayIndex = 0;
		Edge *edge = graph->array[cur].headEdge;
		while (edge)
		{
			int d = edge->dest;
			if (parent[d] == cur)
			{
				sortArray[sortArrayIndex++] = d;
			}
			edge = edge->next;
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

		res.steps[i].recipients = recipients;
		free(sortArray);
	}

	// 释放内存资源
	free(time);
	free(parent);
	free(resQueue);
	free(inDjikstra);
	freeGraph(graph);

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

	// 构建图
	Graph *graph = buildGraph(computers, numComputers, connections, numConnections);
	if (!graph)
	{
		return res;
	}

	// 初始化数据结构
	int *time = (int *)malloc(numComputers * sizeof(int));
	int *poodledTime = (int *)malloc(numComputers * sizeof(int));
	int *currentSecurity = (int *)malloc(numComputers * sizeof(int));
	int *sourceQueue = (int *)malloc(numComputers * sizeof(int));
	int sourceFront = 0, sourceRear = 0;

	for (int i = 0; i < numComputers; i++)
	{
		time[i] = INT_MAX;
		poodledTime[i] = INT_MAX;
		currentSecurity[i] = computers[i].securityLevel;
	}

	// 初始化第一个源计算机
	poodledTime[sourceComputer] = computers[sourceComputer].poodleTime;
	int MaxSourceSecLevel = computers[sourceComputer].securityLevel;

	// 题目给的第一个源计算机入队
	sourceQueue[sourceRear++] = sourceComputer;

	// 处理源计算机队列
	while (sourceFront < sourceRear)
	{
		// 取出sourceQueue队列中的第一个源计算机
		int currentSource = sourceQueue[sourceFront++];
		int sourceSecLevel = currentSecurity[currentSource];

		if (sourceSecLevel > MaxSourceSecLevel)
		{
			for (int i = 0; i < numComputers; i++)
			{
				if (poodledTime[i] > time[i])
				{
					poodledTime[i] = time[i];
				}
				time[i] = INT_MAX;
			}
		}
		time[currentSource] = poodledTime[currentSource];

		// 把源计算机currentSource作为Djikstra的源点u
		bool *inDijkstra = (bool *)calloc(numComputers, sizeof(bool));
		int u = currentSource;

		// 利用Dijkstra算法处理当前源计算机u
		for (int i = 0; i < numComputers; i++)
		{
			inDijkstra[u] = true;

			// 更新u的邻居节点v
			Edge *edge = graph->array[u].headEdge;
			while (edge)
			{
				// v是与u相连接的节点
				int v = edge->dest;
				int newTime = time[u] + edge->transmissionTime + computers[v].poodleTime;

				if (!inDijkstra[v])
				{
					// 1.遇到不大于本轮安全等级sourceSecLevel的节点v
					if (currentSecurity[v] <= sourceSecLevel && newTime < time[v])
					{
						time[v] = newTime;
						currentSecurity[v] = sourceSecLevel;
					}

					// 2.遇到比本轮安全等级sourceSecLevel高1级的节点v
					if (currentSecurity[v] == sourceSecLevel + 1 && newTime < time[v])
					{
						time[v] = newTime;

						// 避免重复入队，造成性能开销
						bool alreadyInQueue = false;
						for (int j = 0; j < sourceRear; j++)
						{
							if (sourceQueue[j] == v)
							{
								alreadyInQueue = true;
								break;
							}
						}
						if (!alreadyInQueue)
						{
							sourceQueue[sourceRear++] = v;
						}
					}
				}
				edge = edge->next;
			}

			// 寻找Djikstra的下一个节点u
			u = -1;
			int minTime = INT_MAX;
			for (int v = 0; v < numComputers; v++)
			{
				if (!inDijkstra[v] && time[v] < minTime)
				{
					minTime = time[v];
					u = v;
				}
			}

			if (u == -1)
				break;
		}

		free(inDijkstra);
		// 本轮Dijkstra结束
	}

	// 释放内存资源
	freeGraph(graph);
	free(currentSecurity);
	free(sourceQueue);
	free(time);

	// 为步骤信息排序
	int stepCount = 0;
	int *resComputer = (int *)malloc(numComputers * sizeof(int));
	int *resTime = (int *)malloc(numComputers * sizeof(int));

	for (int i = 0; i < numComputers; i++)
	{
		if (poodledTime[i] != INT_MAX)
		{
			resComputer[stepCount] = i;
			resTime[stepCount] = poodledTime[i];
			stepCount++;
		}
	}

	// 排序，升序输出
	for (int i = 0; i < stepCount - 1; i++)
	{
		for (int j = 0; j < stepCount - i - 1; j++)
		{
			if (resTime[j] > resTime[j + 1])
			{
				int tempTime = resTime[j];
				resTime[j] = resTime[j + 1];
				resTime[j + 1] = tempTime;

				int tempComputer = resComputer[j];
				resComputer[j] = resComputer[j + 1];
				resComputer[j + 1] = tempComputer;
			}
		}
	}

	res.numSteps = stepCount;
	res.steps = (struct step *)calloc(stepCount, sizeof(struct step));

	// 填充步骤信息
	for (int i = 0; i < stepCount; i++)
	{
		res.steps[i].computer = resComputer[i];
		res.steps[i].time = resTime[i];
		res.steps[i].recipients = NULL;
	}

	// 释放内存资源
	free(poodledTime);
	free(resComputer);
	free(resTime);

	return res;
}
