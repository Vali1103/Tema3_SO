// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
/* TODO: Define graph synchronization mechanisms. */
static pthread_mutex_t graph_mutex; // Declarația mutex-ului global


/* TODO: Define graph task argument. */

void process_graph_node(void *arg)
{
	os_node_t *node = (os_node_t *)arg;

	pthread_mutex_lock(&graph_mutex);

	if (graph->visited[node->id] == PROCESSING) {
		sum += node->info;
		graph->visited[node->id] = DONE;

		for (unsigned int i = 0; i < node->num_neighbours; i++) {
			unsigned int neighbour_idx = node->neighbours[i];

			if (graph->visited[neighbour_idx] == NOT_VISITED) {
				os_node_t *neighbour_node = graph->nodes[neighbour_idx];
				os_task_t *neighbour_task = create_task(process_graph_node, neighbour_node, NULL);

				enqueue_task(tp, neighbour_task);
				graph->visited[neighbour_idx] = PROCESSING;
			}
		}
	}

	pthread_mutex_unlock(&graph_mutex);
}


int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);

	/* TODO: Initialize graph synchronization mechanisms. */
	pthread_mutex_init(&graph_mutex, NULL);

	tp = create_threadpool(NUM_THREADS);

	// primul
	os_node_t *initial_node = graph->nodes[0];
	os_task_t *initial_task = create_task(process_graph_node, initial_node, NULL);

	enqueue_task(tp, initial_task);
	graph->visited[0] = PROCESSING;

	wait_for_completion(tp);

	destroy_threadpool(tp);
	pthread_mutex_destroy(&graph_mutex);


	printf("%d", sum);

	return 0;
}
