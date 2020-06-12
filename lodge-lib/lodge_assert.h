#ifndef _LODGE_ASSERT_H
#define _LODGE_ASSERT_H

#ifdef WIN32

#include <crtdbg.h>

#define ASSERT(expr) \
	_ASSERTE( expr )

#define ASSERT_MESSAGE(expr, message) \
	_ASSERT_EXPR( expr, L#message )

#else

#include <assert.h>

#define ASSERT(expr) \
	assert( (expr) )

#define ASSERT_MESSAGE(expr, message) \
	ASSERT( (expr) && message )

#endif

#define ASSERT_FAIL(message) \
	ASSERT_MESSAGE( 0, message )

#define ASSERT_NOT_IMPLEMENTED() \
	ASSERT_FAIL( "Not implemented" )

#endif
