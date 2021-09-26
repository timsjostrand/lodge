#include "sparse_set.h"

#include "lodge_platform.h"
#include "membuf.h"

#include <string.h>
#include <stdlib.h>

struct sparse_set
{
	char		*dense;

	//
	// Maps `dense_index` => `index`, as in:
	// 
	//     reverse_dense[dense_index] => index
	//
	uint32_t	*reverse_dense;

	size_t		dense_element_size;
	size_t		dense_count_capacity;
	size_t		dense_count;

	uint32_t	*sparse[1024];
	uint32_t	sparse_indices_per_page;
};

sparse_set_t sparse_set_new(size_t dense_element_size, size_t dense_elements_count_default, uint32_t sparse_indices_per_page)
{
	sparse_set_t set = calloc(1, sizeof(struct sparse_set));
	if(!set) {
		return NULL;
	}

	set->dense_element_size = dense_element_size;
	set->dense_count_capacity = dense_elements_count_default;
	set->dense_count = 0;
	set->dense = (char*)malloc(set->dense_count_capacity * set->dense_element_size);
	// TODO(TS): Make reverse_dense optional
	set->reverse_dense = (uint32_t*)malloc(set->dense_count_capacity * sizeof(uint32_t));

	set->sparse_indices_per_page = sparse_indices_per_page;

	return set;
}

void sparse_set_free(sparse_set_t set)
{
	for(uint32_t i = 0; i < 1024; i++) {
		if(set->sparse[i]) {
			free(set->sparse[i]);
		}
	}
	free(set->dense);
	if(set->reverse_dense) {
		free(set->reverse_dense);
	}
	free(set);
}

void* sparse_set_get(sparse_set_t set, uint32_t index)
{
	const uint32_t sparse_index = index / set->sparse_indices_per_page;
	const uint32_t page_index = index % set->sparse_indices_per_page;

	const uint32_t *sparse_page = set->sparse[sparse_index];
	if(!sparse_page) {
		return NULL;
	}

	if(sparse_page[page_index] == 0) {
		return NULL;
	}
	// The dense_index is encoded as `index+1` in the sparse page, as 0 signifies an unused index.
	const uint32_t dense_index = sparse_page[page_index] - 1;

	return &set->dense[dense_index * set->dense_element_size];
}

uint32_t sparse_set_get_index(sparse_set_t set, const void *data)
{
	if(!set->reverse_dense) {
		ASSERT_FAIL("sparse set does not contain reverse lookup table");
		return UINT32_MAX;
	}
	const char *needle = data;
	const char *dense_max = set->dense + set->dense_count * set->dense_element_size;
	if(needle < set->dense || needle > dense_max) {
		ASSERT_FAIL("Data out of bounds");
		return UINT32_MAX;
	}
	const size_t dense_index = (((const char*)needle) - set->dense) / set->dense_element_size;
	return set->reverse_dense[dense_index];
}

void* sparse_set_set_no_init(sparse_set_t set, uint32_t index)
{
	const uint32_t sparse_index = index / set->sparse_indices_per_page;
	const uint32_t page_index = index % set->sparse_indices_per_page;

	uint32_t *sparse_page = set->sparse[sparse_index];
	if(!sparse_page) {
		sparse_page = (uint32_t*)calloc(set->sparse_indices_per_page, sizeof(uint32_t));
		set->sparse[sparse_index] = sparse_page;
	}

	size_t dense_index = sparse_page[page_index];
	if(dense_index == 0) {
		dense_index = set->dense_count++;
		// The dense_index is encoded as `index+1` in the sparse page, as 0 signifies an unused index.
		sparse_page[page_index] = dense_index + 1;
	} else {
		dense_index -= 1;
	}

	if(dense_index >= set->dense_count_capacity) {
		set->dense_count_capacity <<= 1;
		
		char *dense_new = (char*)realloc(set->dense, set->dense_count_capacity * set->dense_element_size);
		ASSERT(dense_new);
		if(dense_new) {
			set->dense = dense_new;
		}

		if(set->reverse_dense) {
			uint32_t *reverse_dense_new = (uint32_t*)realloc(set->reverse_dense, set->dense_count_capacity * sizeof(uint32_t));
			ASSERT(reverse_dense_new);
			if(reverse_dense_new) {
				set->reverse_dense = reverse_dense_new;
			}
		}
	}

	if(set->reverse_dense) {
		set->reverse_dense[dense_index] = index;
	}

	return &set->dense[dense_index * set->dense_element_size];
}

void* sparse_set_set_init_zero(sparse_set_t set, uint32_t index)
{
	void *dst = sparse_set_set_no_init(set, index);
	if(!dst) {
		return NULL;
	}
	memset(dst, 0, set->dense_element_size);
	return dst;
}

size_t sparse_set_get_dense_count(sparse_set_t sparse_set)
{
	return sparse_set ? sparse_set->dense_count : 0;
}

void* sparse_set_set(sparse_set_t set, uint32_t index, const void *src)
{
	void *dst = sparse_set_set_no_init(set, index);
	if(!dst) {
		return NULL;
	}

	memcpy(dst, src, set->dense_element_size);

	return dst;
}

//
// Returns a pointer to the sparse_page where the dense_index is located for this entry index.
// 
// NOTE: The underlying integer is encoded as `dense_index + 1`, so that an underlying value of 0
//       means no dense_index has been assigned.
//
uint32_t* sparse_set_index_to_sparse_page_ptr(sparse_set_t set, uint32_t index)
{
	const uint32_t sparse_index = index / set->sparse_indices_per_page;
	const uint32_t page_index = index % set->sparse_indices_per_page;
	uint32_t *sparse_page = set->sparse[sparse_index];
	if(!sparse_page) {
		return NULL;
	}
	return &sparse_page[page_index];
}

void sparse_set_remove(sparse_set_t set, uint32_t index)
{
	uint32_t *sparse_page_ptr = sparse_set_index_to_sparse_page_ptr(set, index);
	if(!sparse_page_ptr) {
		return;
	}

	size_t dense_index = *sparse_page_ptr;
	if(dense_index == 0) {
		// There is no dense element set -- nothing to do!
		return;
	}
	dense_index -= 1;

	membuf_t buf = membuf_make(set->dense, set->dense_count_capacity * set->dense_element_size, set->dense_element_size);

	size_t index_b = sparse_set_get_index(set, membuf_get(buf, set->dense_count - 1));

	struct membuf_swapret dense_swap_indices = membuf_delete_swap_tail(buf, &set->dense_count, dense_index);

	//
	// Patch the reverse lookup table after swap.
	//
	if(dense_swap_indices.index_b != dense_swap_indices.index_a) {
		if(set->reverse_dense) {
			set->reverse_dense[dense_swap_indices.index_b] = UINT32_MAX;
			set->reverse_dense[dense_swap_indices.index_a] = index_b;
		}
	}

	//
	// Update the location of the swapped element.
	//
	uint32_t *b_sparse_page_ptr = sparse_set_index_to_sparse_page_ptr(set, index_b);
	if(b_sparse_page_ptr) {
		*b_sparse_page_ptr = dense_index + 1;
	}

	//
	// Mark this sparse index as not existing in the dense array anymore.
	//
	*sparse_page_ptr = 0;
}

void* sparse_set_it_begin(sparse_set_t set)
{
	return set->dense_count == 0 ? NULL : &set->dense[0];
}

void* sparse_set_it_next(sparse_set_t set, void *it)
{
	ASSERT((char*)it >= set->dense && (char*)it < (set->dense + set->dense_count * set->dense_element_size));

	char* ptr = ((char*)it) + set->dense_element_size;

	if(ptr >= set->dense + set->dense_count * set->dense_element_size) {
		return NULL;
	}

	return ptr;
}