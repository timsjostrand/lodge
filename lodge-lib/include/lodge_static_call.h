#pragma once

#include <stdio.h>
#include <stdlib.h>

//
// Define a function `f` such that f() will be called before main() but in no
// guaranteed order in comparison to other functions declared by `LODGE_STATIC_CALL`.
//
#ifdef __cplusplus
	#define LODGE_STATIC_CALL(f) \
		static void f(void); \
		struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
		static void f(void)
#elif defined(_MSC_VER)
	#pragma section(".CRT$XCU",read)
	#define LODGE_STATIC_CALL2_(f,p) \
		static void f(void); \
		__declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
		__pragma(comment(linker,"/include:" p #f "_")) \
		static void f(void)
	#ifdef _WIN64
		#define LODGE_STATIC_CALL(f) LODGE_STATIC_CALL2_(f,"")
	#else
		#define LODGE_STATIC_CALL(f) LODGE_STATIC_CALL2_(f,"_")
	#endif
#else
	#define LODGE_STATIC_CALL(f) \
		static void f(void) __attribute__((constructor)); \
		static void f(void)
#endif