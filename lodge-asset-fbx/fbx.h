#ifndef _FBX_H
#define _FBX_H

#include <stdlib.h>

struct fbx;
struct fbx_node;

struct fbx* fbx_new(const char *buf, size_t buf_size);
void		fbx_free(struct fbx *fbx);

void		fbx_print(const struct fbx *fbx);

#endif
