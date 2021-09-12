#include "lodge_time.h"

#include "lodge_assert.h"

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>

#ifdef __MINGW32__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

static uint64_t get_cycles()
{
	return __rdtsc();
}

static double get_seconds()
{
#if 0
	LARGE_INTEGER timestamp;
	LARGE_INTEGER frequency;

	QueryPerformanceCounter(&timestamp);
	QueryPerformanceFrequency(&frequency);

	return (uint64_t)(timestamp.QuadPart / (frequency.QuadPart / 1000));
#else
    uint64_t frequency;
    if(QueryPerformanceFrequency((LARGE_INTEGER*) &frequency)) {
		uint64_t value;
        QueryPerformanceCounter((LARGE_INTEGER*) &value);
        return value / (double)frequency;
	} else {
		ASSERT_NOT_IMPLEMENTED();
		return 0;
	}
#endif
}

static double get_milliseconds()
{
	return get_seconds() * 1000.0f;
}

#else

#include <sys/time.h>

static uint64_t get_cycles()
{
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
}

static double get_milliseconds()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000LL + time.tv_usec / 1000;
}

#endif

double lodge_get_time_ms()
{
	return (double)get_milliseconds();
}

double lodge_get_time_s()
{
	return get_milliseconds() / 1000.0;
}

double lodge_get_time()
{
	return lodge_get_time_s();
}
