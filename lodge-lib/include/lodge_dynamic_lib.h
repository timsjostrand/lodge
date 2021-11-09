#pragma once

#include "lodge_platform.h"

void*	lodge_dynamic_lib_load(const char *filenae);
void*	lodge_dynamic_lib_load_copy(const char *filename, size_t size, void *data);
void*	lodge_dynamic_lib_get_symbol(void *lib, const char *symbol_name);
int		lodge_dynamic_lib_free(void *lib);
