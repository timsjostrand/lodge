/**
 * Linked list.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#ifndef LIST_H
#define	LIST_H

/**
 * A linked list.
 */
struct list;

/**
 * An element contained in a linked list.
 */
struct element;

struct list*    list_new();
void            list_free(struct list *list, int free_data);
int             list_prepend(struct list *list, void *data);
int             list_append(struct list *list, void *data);
int             list_empty(struct list *list);
int             list_count(struct list *list);
void            list_clear(struct list *list, int free_data);
struct element* list_head(struct list *list);
struct element* list_first(struct list *list);
struct element* list_last(struct list *list);
int             list_insert_element_at(struct list *list, int index, struct element *e);
int             list_insert_at(struct list *list, int index, void *data);
int             list_index_of(struct list *list, void *data);
struct element* list_get_element(struct list *list, void *data);
struct element* list_element_at(struct list *list, int index);
void            list_element_init(struct element *e, void *data);
struct element* list_element_new(void *data);
void            list_element_free(struct element *e, int free_data);
void            list_element_delete(struct element *e, int free_data);
int             list_init(struct list *list);
void            list_prepend_element(struct list *list, struct element *e);
void            list_append_element(struct list *list, struct element *e);
void            list_print_char(struct list *list);

struct list*	list_copy(struct list *original);
struct list*	list_copy_subset(struct list *original, size_t start, size_t count);

/**
 * Iterates a list of the specified type and stores the currently iterated
 * element in a variable named after 'item'.
 *
 * @code
 * foreach_list(struct char_element*, e, list) {
 *     printf("%d=%s\n", index, e->data);
 * }
 * @endcode
 * 
 * @param type  The pointer type of elements in the list.
 * @param item  The name assigned to the currently iterated element pointer.
 * @param list  The list to iterate over.
 */
#define foreach_list(type, item, list) \
	for(type item = (type) list_first(list); \
                (item != NULL) && ((struct element*) item != list_head(list)); \
                item = (type) item->next)

/**
 * Iterates a list in reverse.
 * 
 * @see foreach_list(type, item, list)
 */
#define foreach_reverse(type, item, list) \
	for(type item = (type) list_last(list); \
                (item != NULL) && ((struct element*) item != list_head(list)); \
                item = (type) item->prev)

/**
 * Defines a new list element where 'data' is a pointer of the specified type.
 * 
 * Example: @code define_element(struct char_element, char *); @endcode
 * 
 * Results in: @code
 * struct char_element {
 *	char *data;
 *	struct element *next;
 *	struct element *prev;
 * };
 * @endcode
 * 
 * @param name  The struct declaration including the name for the element.
 * @param type  The data pointer type.
 */
#define define_element(name, type) \
        name { \
		type data; \
		struct element *next; \
		struct element *prev; \
	}

define_element(struct char_element, char *);

#endif	/* LIST_H */

