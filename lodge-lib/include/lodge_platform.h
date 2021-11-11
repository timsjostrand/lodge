#pragma once

#include "lodge_assert.h"

//
// Numeric data type helpers (size_t, uint{8,16,32,64}_t, FLT_{MIN,MAX}).
//
#include <stddef.h>
#include <stdint.h>
#include <float.h>

//
// The `bool` type.
//
#include <stdbool.h>

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

//
// Compile-time static array size.
//
#if defined(_MSC_VER)
	#define lodge_countof(a) (_countof(a))
#else
	#define lodge_countof(a) (sizeof(a) / sizeof(a[0]))
#endif
#define LODGE_ARRAYSIZE(a) lodge_countof(a)

//
// Flag & bit helpers.
//
#define LODGE_BIT(n) 1 << (n-1)
#define LODGE_IS_FLAG_SET(FIELD, FLAG) ((FIELD & FLAG) == FLAG)

//
// sizeof for struct members.
//
#define sizeof_member(type, member) sizeof(((type *)0)->member)

//
// Explicitly allow unused symbol (eg. for use in ASSERT)
//
#define LODGE_UNUSED(x) ((void)(x))

//
// Stack dynamic allocations.
//
#ifdef _WIN32
	#include <malloc.h>
	#define LODGE_ALLOCA _alloca
#endif

#ifdef linux
	#include <alloca.h>
	#define LODGE_ALLOCA alloca
#endif

//
// Dynamic library export symbol.
//
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

#ifndef min
	#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
	#define max(x, y) ((x) > (y) ? (x) : (y))
#endif
