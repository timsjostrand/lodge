#include "lodge_time.h"

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

static uint64_t get_milliseconds()
{
	LARGE_INTEGER timestamp;
	LARGE_INTEGER frequency;

	QueryPerformanceCounter(&timestamp);
	QueryPerformanceFrequency(&frequency);

	return (uint64_t)(timestamp.QuadPart / (frequency.QuadPart / 1000));
}

#else

#include <sys/time.h>

static uint64_t get_cycles()
{
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
}

static uint64_t get_milliseconds()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000LL + time.tv_usec / 1000;
}

#endif


double lodge_get_time()
{
	return get_milliseconds() / 1000.0f;
}