#ifndef _LODGE_PLATFORM_H
#define _LODGE_PLATFORM_H

#include "lodge_assert.h"
#include "lodge_time.h"

#include <stddef.h> // size_t
#include <stdint.h>
#include <float.h>

#ifdef _WIN32
#define EXPORT __declspec( dllexport )
#define IMPORT __declspec( dllimport )
#else
#define EXPORT
#define IMPORT
#endif

#ifdef ENABLE_SHARED
#define SHARED_SYMBOL EXPORT
#else

#ifdef LOAD_SHARED
#define SHARED_SYMBOL IMPORT
#else
#define SHARED_SYMBOL
#endif

#endif

#define LODGE_ARRAYSIZE(a) ( sizeof(a) / sizeof(a[0]) )

#define LODGE_BIT(n) 1 << n

#define sizeof_member(type, member) sizeof(((type *)0)->member)

double	lodge_get_time();

void*	lodge_lib_load(const char *filenae);
void*	lodge_lib_load_copy(const char *filename, size_t size, void *data);
void*	lodge_lib_get_symbol(void *lib, const char *symbol_name);
int		lodge_lib_free(void *lib);

#endif