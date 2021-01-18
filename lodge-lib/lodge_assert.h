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

#define LODGE_TOKEN_PASTE(x, y) x ## y
#define LODGE_TOKEN_PASTE2(x, y) LODGE_TOKEN_PASTE(x, y)

#define LODGE_VAR_NAME(name) LODGE_TOKEN_PASTE2(name, __LINE__)

//
// `ASSERT_OR()` is a convenience macro that evaluates an expression, ASSERTs
// if false (if ASSERT is enabled) and additionally runs the scope immediately
// following the macro if the evaluation failed.
// 
// Note that the scope is always run if the expression fails, regardless if
// ASSERTs are enabled or not.
// 
// Using ASSERT_OR it is possible to write debuggable flow statements such as:
// 
//     FILE *f = fopen("tmp.txt", "r");
//     ASSERT_OR(f) {
//         return 0;
//     }
// 
//     bool ret = fseek(f, 0, SEEK_SET);
//     ASSERT_OR(ret) {
//         return 0;
//     }
//
#if 1
	#define ASSERT_OR(expr) \
		_Bool LODGE_VAR_NAME(assert_or_ret_) = (_Bool)(expr); ASSERT_MESSAGE(LODGE_VAR_NAME(assert_or_ret_), #expr); if(!(LODGE_VAR_NAME(assert_or_ret_)))
#else
	// 
	// Better ASSERT_OR that only runs `expr` once and in a closed scope, but does not allow `continue` or `break` :(
	//
	#define ASSERT_OR( expr ) \
		for(int LODGE_VAR_NAME(ASSERT_OR_RET) = (expr), LODGE_VAR_NAME(ASSERT_OR_DUMMY) = (ASSERT_MESSAGE(LODGE_VAR_NAME(ASSERT_OR_RET), #expr), 0); LODGE_VAR_NAME(ASSERT_OR_RET) == 0; LODGE_VAR_NAME(ASSERT_OR_RET)--)
#endif

#endif
