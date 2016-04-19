#ifndef RQUEUE_H
#define RQUEUE_H

#include <stdlib.h>

/**
 * If defined, new elements will be initialized to NULL, which decreases
 * performance but makes debugging easier.
 */
#define RQUEUE_INIT_ELEMENTS

typedef void rqueue_data_t;

/**
 * A dynamic ring queue.
 */
struct rqueue {
	size_t			capacity;   /* Size of elements array. */
	size_t			count;      /* How many elements currently in list. */
	rqueue_data_t	**data;     /* Data pointer array. */
    size_t			front;		/* The front of the queue. */
    signed long		back;		/* The back of the queue.. */
};

struct rqueue*	rqueue_new(size_t initial_capacity);
int				rqueue_init(struct rqueue *rqueue, size_t initial_capacity);
void			rqueue_free(struct rqueue *rqueue, int free_data);
int				rqueue_resize(struct rqueue *rqueue, size_t capacity);

int				rqueue_push(struct rqueue *rqueue, rqueue_data_t *data);
rqueue_data_t*	rqueue_pop(struct rqueue *rqueue);
rqueue_data_t*	rqueue_peek(struct rqueue *rqueue);
void			rqueue_clear(struct rqueue *rqueue, int free_data);
int				rqueue_empty(struct rqueue *rqueue);
size_t			rqueue_count(struct rqueue *rqueue);
size_t			rqueue_inc(struct rqueue *rqueue, size_t index);

#if 0
struct rqueue*	rqueue_copy(struct rqueue *original);
struct rqueue*	rqueue_copy_subset(struct rqueue *original, size_t start, size_t count);
#endif

void			rqueue_print_strings(struct rqueue *rqueue);
void			rqueue_dump_strings(struct rqueue *rqueue);

/**
 * Iterates a queue of the specified type and stores the currently iterated
 * element in a variable named after 'item'.
 *
 * @code
 * foreach_rqueue(char*, s, index, queue) {
 *	   printf("%d=%s\n", index, s);
 * }
 * @endcode
 *
 * This macro reserves the name 'rqueue_counter' for a counter of the natural
 * ordered index of the currently iterated element.
 *
 * @param type		The pointer type of elements in the queue.
 * @param item		The name assigned to the currently iterated element pointer.
 * @param index		The name of the counter variable (probably 'i' or index').
 * @param queue		The queue to iterate over.
 */
#define foreach_rqueue(type, item, index, queue) \
	for(size_t index = queue->front, rqueue_counter = 0; \
			rqueue_counter < queue->count; \
			index = rqueue_inc(queue, index), rqueue_counter ++) \
	for(type item = (type) queue->data[index]; \
			item != (type) &queue->capacity; \
			item = (type) &queue->capacity)

/**
 * Pop all the elements in the queue.
 *
 * @param type		The pointer type of elements in the queue.
 * @param item		The name assigned to the currently iterated element pointer.
 * @param queue		The queue to iterate over.
 */
#define rqueue_consume(type, item, queue) \
	for(type item = NULL; \
			queue->count > 0 && (item = (type) rqueue_pop(queue)) != (type) queue; ) \

#endif
