#ifndef _LODGE_ASSERT_H
#define _LODGE_ASSERT_H

#ifdef WIN32

#include <crtdbg.h>

#define ASSERT(expr, message) \
	_ASSERT_EXPR( expr, L#message )

#define ASSERT_FAIL(message) \
	_ASSERT_EXPR( 0, L#message )

#else

#include <assert.h>

#define ASSERT(expr, message) \
	assert( (expr) && (message) )

#define ASSERT_FAIL(message) ASSERT( 0, (message) )

#endif

#endif