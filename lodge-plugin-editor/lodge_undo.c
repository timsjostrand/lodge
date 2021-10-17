#include "lodge_undo.h"

struct lodge_undo_frame
{
	lodge_undo_func_t			undo_func;
	lodge_undo_func_t			redo_func;
	void						*data;
};

struct lodge_undo
{
	size_t						current_frame;

	struct lodge_undo_frame		*frames;
};

void lodge_undo_add(struct lodge_undo *undo, lodge_undo_func_t undo_func, lodge_undo_func_t redo_func, const void *data, size_t data_size)
{
	undo->frames[undo->current_frame++].
}