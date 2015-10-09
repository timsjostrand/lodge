/**
 * A linked list.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

struct list {
	struct element *head;
};

struct element {
	void *data;
	struct element *next;
	struct element *prev;
};

/**
 * Initializes an allocated list element with the specified data.
 *
 * @param e	The element to initialize.
 * @param data	The data to assign the element.
 * @see		list_element_new
 */
void list_element_init(struct element *e, void *data)
{
	memset(e, 0, sizeof(*e));
	e->data = data;
	e->next = NULL;
	e->prev = NULL;
}

/**
 * Allocates and initializes a new list element with the specified data.
 *
 * @param data	The data to assign to the new element.
 * @return	A pointer to the new element.
 * @see		list_element_free()
 */
struct element* list_element_new(void *data)
{
	struct element *tmp = malloc(sizeof(struct element));

	if(tmp == NULL) {
		return NULL;
	}

	list_element_init(tmp, data);

	return tmp;
}

/**
 * Free an element but do not touch the links pertaining to it.
 *
 * @param e		The element to free.
 * @param free_data	Whether or not to also free the contained data.
 */
void list_element_free(struct element *e, int free_data)
{
	if(free_data) {
		free(e->data);
	}
	free(e);
}

/**
 * Unlink and free an element.
 *
 * @param e		The element to unlink and free.
 * @param free_data	Whether or not to also free the contained data.
 */
void list_element_delete(struct element *e, int free_data)
{
	/* Unlink element from list. */
	if(e->next != NULL) {
		e->next->prev = e->prev;
	}
	if(e->prev != NULL) {
		e->prev->next = e->next;
	}
	/* Free element memory. */
	list_element_free(e, free_data);
}

/**
 * Creates the head element and initializes a list.
 *
 * @param list	The list to initialize.
 * @return	True if successful, false if out of memory.
 */
int list_init(struct list *list)
{
	memset(list, 0, sizeof(struct list));

	/* Allocate a new head element. */
	list->head = list_element_new(NULL);
	/* Out of memory? */
	if(list->head == NULL) {
		return 0;
	}

	/* No more elements at the moment. */
	list->head->next = list->head;
	list->head->prev = list->head;

	return 1;
}

/**
 * @return A new, empty list or NULL if out of memory.
 * @see list_free
 */
struct list* list_new()
{
	struct list* tmp = malloc(sizeof(struct list));

	if(tmp == NULL) {
		return NULL;
	}

	if(!list_init(tmp)) {
		free(tmp);
		return NULL;
	}

	return tmp;
}

/**
 * Removes all the elements contained in a list.
 *
 * @param list		The list to clear.
 * @param free_data	Whether or not to also free the data contained in each
 *			element.
 */
void list_clear(struct list *list, int free_data)
{
	/* NOTE: Cannot use the 'foreach_list' construct here because it expects
	 *       item->next to be present when leaving the loop scope. */
	struct element *elem = NULL;
	while((elem = list_first(list)) != NULL && (elem != list->head)) {
		list_element_delete(elem, free_data);
	}
}

/**
 * Deletes a list and all the elements contained in it.
 * 
 * @param list		The list to delete.
 * @param free_data	Whether or not to also free the data contained in each
 *			element.
 */
void list_free(struct list *list, int free_data)
{
	/* Remove all elements in list. */
	list_clear(list, free_data);
	/* Also remove the head element. */
	list_element_free(list->head, free_data);
	/* Free the list itself. */
	free(list);
}

/**
 * @param list	The list to get the head element of.
 * @return	Returns the head element of the list, equivalent to calling
 *		list->head.
 */
struct element* list_head(struct list *list)
{
	return list->head;
}

/**
 * @param list	The list to return the first element from.
 * @return	The first element in the list, or the head element if the list
 *		is empty.
 */
struct element* list_first(struct list *list)
{
	struct element *first = list->head->next;

	if(first == NULL) {
		first = list->head;
	}

	return first;
}

/**
 * @param list	The list to return the last element from.
 * @return	The last element in the list, or the head element if the list
 *		is empty.
 */
struct element* list_last(struct list *list)
{
	struct element *last = list->head->prev;

	if(last == NULL) {
		last = list->head;
	}

	return last;
}

/**
 * @param list	The list the check for emptiness.
 * @return	Checks if the list is empty.
 */
int list_empty(struct list *list)
{
	return (list_last(list) == list->head);
}

/**
 * Counts all the elements of the list (expensive!).
 *
 * @param list	The list to count the elements of.
 * @return	The number of elements in the list.
 */
int list_count(struct list *list)
{
	int count = 0;
	foreach_list(struct str_element*, elem, list) {
		count ++;
	}
	return count;
}

/**
 * Inserts an element first in a list (immediately after the head element).
 *
 * @param list	The list to add the element to.
 * @param e	The element to add.
 */
void list_prepend_element(struct list *list, struct element *e)
{
	struct element *first = list_first(list);
	e->next = first;
	e->prev = first->prev;
	first->prev->next = e;
	first->prev = e;
}

/**
 * Creates a list element with the specified data and inserts it first in a
 * list.
 *
 * @param list	The list to prepend the element into.
 * @param data	The data to assign to the new element.
 * @return	True when successful, false if out of memory.
 */
int list_prepend(struct list *list, void *data)
{
	struct element *e = list_element_new(data);

	if(e == NULL) {
		return 0;
	}

	list_prepend_element(list, e);

	return 1;
}

/**
 * Inserts an element last in a list.
 *
 * @param list	The list to add the element to.
 * @param e	The element to add.
 */
void list_append_element(struct list *list, struct element *e)
{
	struct element *last = list_last(list);
	struct element *last_old_next = last->next;

	e->next = last_old_next;	/* This is list->head. */
	e->prev = last;
	last->next = e;
	last_old_next->prev = e;
}

/**
 * Creates a list element with the specified data and inserts it last in a list.
 *
 * @param list	The list to add the element to.
 * @param data	The data to assign to the new element.
 * @return	True if successful, false if out of memory.
 */
int list_append(struct list *list, void *data)
{
	struct element *e = list_element_new(data);

	if(e == NULL) {
		return 0;
	}

	list_append_element(list, e);

	return 1;
}

/**
 * Inserts an element at the specified index in the list. This performs poorly
 * and where possible list_append() and list_prepend() should be used instead.
 *
 * If index is 0, the result will be equivalent to list_prepend().
 *
 * If index is >= the length of the list, nothing is added.
 *
 * @param list	The list to perform the insert in.
 * @param index	The position at which to insert the element in the list.
 * @param e	The element to insert.
 * @see		list_prepend_element(list, e)
 * @see		list_append_element(list, e)
 * @return	The index assigned to the element. Notably, this index will
 *		differ from the specified one if the list does not contain
 *		so many elements.
 */
int list_insert_element_at(struct list *list, int index, struct element *e)
{
	struct element *curr = list_element_at(list, index);

	/* Sanity check. */
	if(curr == NULL) {
		/* Specified index beyond current range, or the list is empty.
		 * In either case: append element last in list instead. */
		list_append_element(list, e);
		return list_count(list);
	} else {
		/* Prepend to curr. */
		e->next = curr;
		e->prev = curr->prev;
		curr->prev->next = e;
		curr->prev = e;
		return index;
	}
}

int list_insert_at(struct list *list, int index, void *data)
{
	struct element *e = list_element_new(data);

	if(e == NULL) {
		return 0;
	}

	if(!list_insert_element_at(list, index, e)) {
		/* TODO: free_data should be per list instead, this is not clean... */
		list_element_free(e, 0);
		return 0;
	}

	return 1;
}

int list_index_of(struct list *list, void *data)
{
	int index = 0;
	foreach_list(struct element*, e, list) {
		if(e->data == data) {
			return index;
		}
		index ++;
	}
	return -1;
}

struct element* list_element_at(struct list *list, int index)
{
	int i = 0;
	foreach_list(struct element *, e, list) {
		if(i == index) {
			return e;
		}
		i ++;
	}
	return NULL;
}

struct element* list_get_element(struct list *list, void *data)
{
	foreach_list(struct element *, e, list) {
		if(e->data == data) {
			return e;
		}
	}
	return NULL;
}

/**
 * A utility function to print a list of strings (struct str_element) to
 * stdout on the form "index=string".
 *
 * @param list	The list to print.
 */
void list_print_strings(struct list *list)
{
	int index = 0;
	foreach_list(struct str_element*, elem, list) {
		printf("%d=%s\n", index, elem->data == NULL ? "NULL" : elem->data);
		index ++;
	}
}

/**
 * Creates a new list with the same set of elements as another list. This new
 * list must be freed separately.
 *
 * Care must be taken when freeing copied lists: note that the new list will
 * have separately allocated element structures (can be freed independently)
 * but that the data contents of these elements are _not_ separately allocated!
 *
 * @param original	The original list to make a copy of.
 * @return			A copy of the original list.
 */
struct list* list_copy(struct list *original)
{
	struct list *tmp = list_new();
	foreach_list(struct element *, e, original) {
		list_append(tmp, e->data);
	}
	return tmp;
}

/**
 * Copies a list into a new one like list_copy(), but select only a subset of
 * the elements.
 *
 * @see list_copy
 */
struct list* list_copy_subset(struct list *original, size_t start, size_t count)
{
	struct list *tmp = list_new();
	size_t index = 0;
	foreach_list(struct element *, e, original) {
		if(index > start+count) {
			break;
		}
		if(index >= start) {
			list_append(tmp, e->data);
		}
		index ++;
	}
	return tmp;
}
