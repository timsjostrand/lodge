#include "vfs.h"
#include "core.h"
#include "assets.h"

/* Core singleton. */
struct core core_mem = { 0 };
struct core *core_global = &core_mem;

/* VFS singleton. */
struct vfs vfs_mem = { 0 };
struct vfs *vfs_global = &vfs_mem;

/* Input singleton. */
struct input *input_global = NULL;

/* Assets singleton. */
struct assets assets_mem = { 0 };
struct assets *assets = &assets_mem;
