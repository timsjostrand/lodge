#ifndef ALIST_H
#define ALIST_H

typedef void alist_data_t;

/**
 * A dynamic array List.
 */
struct alist {
	size_t			size;	/* Size of elements array. */
	size_t			count;	/* How many elements currently in list. */
	alist_data_t	**data;	/* Data pointer array. */
};

struct alist*	alist_new(size_t initial_size);
int				alist_init(struct alist *alist, size_t initial_size);
void			alist_free(struct alist *alist, int free_data);
int				alist_resize(struct alist *alist, size_t size);

int				alist_prepend(struct alist *alist, alist_data_t *data);
int				alist_append(struct alist *alist, alist_data_t *data);
void			alist_clear(struct alist *alist, int free_data);
alist_data_t*	alist_get(struct alist *alist, size_t index);
int				alist_insert_at(struct alist *alist, size_t index, alist_data_t *data);
alist_data_t*	alist_delete_at(struct alist *alist, size_t index, int free_data);
int				alist_empty(struct alist *alist);
size_t			alist_count(struct alist *alist);
alist_data_t*	alist_first(struct alist *alist);
alist_data_t*	alist_last(struct alist *alist);
int				alist_index_of(struct alist *alist, alist_data_t *data, size_t *index);

struct alist*	alist_copy(struct alist *original);
struct alist*	alist_copy_subset(struct alist *original, size_t start, size_t count);

void			alist_print_strings(struct alist *alist);

/**
 * Iterates a list of the specified type and stores the currently iterated
 * element in a variable named after 'item'.
 *
 * @code
 * foreach_alist(char*, s, index, list) {
 *	   printf("%d=%s\n", index, s);
 * }
 * @endcode
 *
 * @param type		The pointer type of elements in the list.
 * @param item		The name assigned to the currently iterated element pointer.
 * @param counter	The name of the counter variable (probably 'i' or index').
 * @param list		The list to iterate over.
 */
#define foreach_alist(type, item, counter, list) \
	for(size_t counter = 0; \
			counter < list->count; \
			counter ++) \
	for(type item = (type) list->data[counter]; \
			item != (type) &list->size; \
			item = (type) &list->size)

/**
 * This is a slightly more optimized version of foreach_alist implemented
 * using pointer arithmetic, but is less user-friendly since it requires the
 * data pointer to be manually dereferenced before use. As a feature, it does
 * not require/provide an index variable.
 *
 * Example:
 * @code
 * foreach_alist_p(char*, s, list) {
 *     printf("%s\n", (*s));
 * }
 * @endcode
 *
 * @param type	Type of pointers contained in the list.
 * @param item	The name of the currently iterated item.
 * @param list	The list to iterate.
 */
#define foreach_alist_p(type, item, list) \
	for(type *item = (type *) list->data; \
			item < ((type *) list->data) + list->count; \
			item++)

#endif
