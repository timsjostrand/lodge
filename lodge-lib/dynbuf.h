#ifndef _DYNBUF_H
#define _DYNBUF_H

#include <stddef.h>

typedef struct dynbuf
{
	size_t		*capacity;
	size_t		*count;
	size_t		type_size;
	char		**ptr;
} dynbuf_t;


//
//	Can be used as on a struct that implements the dynbuf "interface":
//
//		struct my_buf
//		{
//			size_t			count;
//			size_t			capacity;
//			struct my_elem	*elements;
//		} my_buf_global;
//
//		struct my_elem foo = { 0 };
//		dynbuf_append(dynbuf_wrap(&my_buf_global), &foo, sizeof(foo));
//	
#define			dynbuf_ptr(DYNBUF) (dynbuf_t) { .capacity = &DYNBUF->capacity, .count = &DYNBUF->count, .type_size = sizeof(*DYNBUF->elements), .ptr = (char **)&DYNBUF->elements }
#define			dynbuf(DYNBUF) dynbuf_ptr( (&DYNBUF) )

// Deprecated API:
#define			dynbuf_wrap(DYNBUF) dynbuf_ptr(DYNBUF)
#define			dynbuf_wrap_stack(DYNBUF_STACK) dynbuf(DYNBUF_STACK)

void			dynbuf_new_inplace(dynbuf_t dynbuf, size_t default_capacity);
void			dynbuf_free_inplace(dynbuf_t dynbuf);

void			dynbuf_clear(dynbuf_t dynbuf);

void*			dynbuf_append(dynbuf_t dst, const void *src, size_t src_size);
void*			dynbuf_append_no_init(dynbuf_t dst);
void*			dynbuf_append_range(dynbuf_t dst, void *src, size_t src_type_size, size_t src_count);

#endif