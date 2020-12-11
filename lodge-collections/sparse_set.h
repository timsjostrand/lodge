#ifndef _SPARSE_SET_H
#define _SPARSE_SET_H

#include <stdint.h>

struct sparse_set;
typedef struct sparse_set* sparse_set_t;

sparse_set_t	sparse_set_new(size_t dense_element_size, size_t dense_elements_count_default, uint32_t sparse_indices_per_page);
void			sparse_set_free(sparse_set_t sparse_set);

void*			sparse_set_get(sparse_set_t sparse_set, uint32_t index);
uint32_t		sparse_set_get_index(sparse_set_t set, const void *data);
void*			sparse_set_set(sparse_set_t sparse_set, uint32_t index, const void *src);
void*			sparse_set_set_no_init(sparse_set_t set, uint32_t index);

size_t			sparse_set_get_dense_count(sparse_set_t sparse_set);

//
// Example:
//
//		for(float *it = sparse_set_it_begin(set); it; it = sparse_set_it_next(set, it))
//			printf("%f\n", *it);
//
void*			sparse_set_it_begin(sparse_set_t set);
void*			sparse_set_it_next(sparse_set_t set, void *it);

#endif