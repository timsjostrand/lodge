#ifndef _LODGE_UNDO_H
#define _LODGE_UNDO_H

#include <stdint.h>

struct lodge_undo;

typedef void (*lodge_undo_func_t)(void *data);

void lodge_undo_new_inplace(struct lodge_undo *undo);
void lodge_undo_free_inplace(struct lodge_undo *undo);

void lodge_undo_add(struct lodge_undo *undo, lodge_undo_func_t undo_func, lodge_undo_func_t redo_func, const void *data, size_t data_size);

void lodge_undo_undo(struct lodge_undo *undo);
void lodge_undo_redo(struct lodge_undo *undo);

#endif