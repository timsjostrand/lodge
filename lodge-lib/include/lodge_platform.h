#ifndef _LODGE_PLATFORM_H
#define _LODGE_PLATFORM_H

#include "lodge_assert.h"
#include "lodge_time.h"

#include <stddef.h> // size_t
#include <stdint.h>
#include <stdbool.h>
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

#if defined(_MSC_VER)
	#define lodge_countof(a) (_countof(a))
#else
	#define lodge_countof(a) (sizeof(a) / sizeof(a[0]))
#endif
#define LODGE_ARRAYSIZE(a) lodge_countof(a)

#define LODGE_BIT(n) 1 << (n-1)
#define LODGE_IS_FLAG_SET(FIELD, FLAG) ((FIELD & FLAG) == FLAG)

#define sizeof_member(type, member) sizeof(((type *)0)->member)

#define LODGE_UNUSED(x) ((void)(x))

#ifdef _WIN32
#include <malloc.h>
#define LODGE_ALLOCA _alloca
#endif

//
// LODGE_DEPRECATED marks a function as deprecated using compiler hints where available.
// 
// Usage:
// 
//     LODGE_DEPRECATED void foo(int a);
//
#if defined(__GNUC__) || defined(__clang__)
#define LODGE_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define LODGE_DEPRECATED __declspec(deprecated)
#else
#pragma message("LODGE_DEPRECATED not implemented for this compiler")
#define LODGE_DEPRECATED
#endif

void*	lodge_lib_load(const char *filenae);
void*	lodge_lib_load_copy(const char *filename, size_t size, void *data);
void*	lodge_lib_get_symbol(void *lib, const char *symbol_name);
int		lodge_lib_free(void *lib);

#endif