/**
 * Dynamic ring queue implementation.
 *
 * Properties:
 * - FIFO
 * - Ordered.
 * - Constant performance.
 * - Wraps around containing buffer if allowed.
 * - Expands in increments of 2x when full.
 *
 *      Back   Front
 *       |       |
 * +---+-+-+---+-+-+---+---+
 * | D | E | - | A | B | C |
 * +---+---+---+---+---+---+
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rqueue.h"

static int rqueue_grow(struct rqueue *rqueue)
{
	if(rqueue == NULL) {
		return -1;
	}
	if(rqueue_resize(rqueue, rqueue->capacity * 2) != 0) {
		return -1;
	}
	return 0;
}

static void rqueue_inc_back(struct rqueue *rqueue)
{
	rqueue->back = (++ rqueue->back) % rqueue->capacity;
}

size_t rqueue_inc(struct rqueue *rqueue, size_t index)
{
	return (++ index) % rqueue->capacity;
}

struct rqueue* rqueue_new(size_t initial_capacity)
{
	struct rqueue *tmp = (struct rqueue *) malloc(sizeof(struct rqueue));
	if(rqueue_init(tmp, initial_capacity) != 0) {
		goto cleanup;
	}
	return tmp;
cleanup:
	free(tmp);
	return NULL;
}

static void rqueue_free_data_elements(struct rqueue *rqueue)
{
	foreach_rqueue(void *, e, index, rqueue) {
		free(e);
	}
}

int rqueue_init(struct rqueue *rqueue, size_t initial_capacity)
{
	if(rqueue == NULL) {
		return -1;
	}

	/* Sanity check. */
	if(initial_capacity == 0) {
		return -1;
	}

#ifdef RQUEUE_INIT_ELEMENTS
	rqueue->data = (rqueue_data_t *) calloc(initial_capacity, sizeof(rqueue_data_t *));
#else
	rqueue->data = (rqueue_data_t *) malloc(initial_capacity * sizeof(rqueue_data_t *));
#endif

	if(rqueue->data == NULL) {
		return -1;
	}

	rqueue->capacity = initial_capacity;
	rqueue->count = 0;
	rqueue->front = 0;
	rqueue->back = -1;

	return 0;
}

void rqueue_free(struct rqueue *rqueue, int free_data)
{
	if(free_data) {
		rqueue_free_data_elements(rqueue);
	}
	free(rqueue->data);
	free(rqueue);
}

int rqueue_resize(struct rqueue *rqueue, size_t new_capacity)
{
#ifdef RQUEUE_INIT_ELEMENTS
	rqueue_data_t **tmp = (rqueue_data_t **) calloc(new_capacity, sizeof(rqueue_data_t *));
#else
	rqueue_data_t **tmp = (rqueue_data_t **) malloc(new_capacity * sizeof(rqueue_data_t *));
#endif
	if(tmp == NULL) {
		return -1;
	}
	/* Reorder queue. */
	foreach_rqueue(rqueue_data_t *, elem, index, rqueue) {
		tmp[rqueue_counter] = elem;
	}
	free(rqueue->data);

	rqueue->data = tmp;
	rqueue->front = 0;
	rqueue->back = rqueue->count - 1;
	rqueue->capacity = new_capacity;
	return 0;
}

void rqueue_clear(struct rqueue *rqueue, int free_data)
{
	if(free_data) {
		rqueue_free_data_elements(rqueue);
	}
	rqueue->count = 0;
	rqueue->front = 0;
	rqueue->back = -1;
}

int rqueue_empty(struct rqueue *rqueue)
{
	return rqueue->count == 0;
}

size_t rqueue_count(struct rqueue *rqueue)
{
	return rqueue->count;
}

int rqueue_push(struct rqueue *rqueue, rqueue_data_t *data)
{
	if(rqueue->count >= rqueue->capacity) {
		if(rqueue_resize(rqueue, rqueue->capacity * 2) != 0) {
			return -1;
		}
	}
	rqueue_inc_back(rqueue);
	rqueue->data[rqueue->back] = data;
	rqueue->count ++;
	return 0;
}

/**
 * Remove and return the last inserted element from the queue.
 *
 * @param rqueue	The queue to pop from.
 *
 * @return			The last inserted element or NULL if queue is empty.
 */
rqueue_data_t* rqueue_pop(struct rqueue *rqueue)
{
	if(rqueue->count == 0) {
		return NULL;
	}
	rqueue_data_t *tmp = rqueue->data[rqueue->front];
#ifdef RQUEUE_INIT_ELEMENTS
	rqueue->data[rqueue->front] = NULL;
#endif
	rqueue->front = rqueue_inc(rqueue, rqueue->front);
	rqueue->count --;
	return tmp;
}

/**
 * Return the last inserted element in the queue without removing it.
 *
 * @param rqueue	The queue to peek in.
 *
 * @return			The last inserted element or NULL if queue is empty.
 */
rqueue_data_t* rqueue_peek(struct rqueue *rqueue)
{
	if(rqueue->count == 0) {
		return NULL;
	}
	return rqueue->data[rqueue->front];
}

/**
 * A debug function that assumes all pointers in a list are const char
 * pointers, and prints them to stdout with the format "index=string".
 */
void rqueue_print_strings(struct rqueue *rqueue)
{
	foreach_rqueue(const char *, s, index, rqueue) {
		printf("%zu=%s\n", index, s);
	}
}

/**
 * A debug function that assumes all pointers in a list are const char
 * pointers and prints them in the order they appear in the backing buffer,
 * including an 'f' or 'b' if the element is the front or back of the queue.
 */
void rqueue_dump_strings(struct rqueue *rqueue)
{
	for(size_t i=0; i<rqueue->capacity; i++) {
		printf("%c %zu=%s\n", i == rqueue->front ? 'f' : i == rqueue->back ? 'b' : ' ', i, rqueue->data[i]);
	}
}
