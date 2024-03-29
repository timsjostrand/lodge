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

#if 0
static uint64_t get_cycles()
{
	return __rdtsc();
}
#endif

static double get_seconds()
{
	LARGE_INTEGER timestamp;
	LARGE_INTEGER frequency;
	QueryPerformanceCounter(&timestamp);
	QueryPerformanceFrequency(&frequency);
	return (uint64_t)(timestamp.QuadPart / frequency.QuadPart);
}


lodge_timestamp_t lodge_timestamp_get()
{
	_Static_assert(sizeof(((LARGE_INTEGER *)0)->QuadPart) == sizeof(lodge_timestamp_t), "lodge_timestamp_t is too small to fit platform timestamp");
	LARGE_INTEGER timestamp;
	QueryPerformanceCounter(&timestamp);
	return timestamp.QuadPart;
}

double lodge_timestamp_elapsed_s(lodge_timestamp_t before)
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER timestamp;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&timestamp);

	LARGE_INTEGER elapsed;
	elapsed.QuadPart = timestamp.QuadPart - before;
	elapsed.QuadPart /= frequency.QuadPart;

	return elapsed.QuadPart;
}

double lodge_timestamp_elapsed_ms(lodge_timestamp_t before)
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER timestamp;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&timestamp);

	LARGE_INTEGER elapsed;
	elapsed.QuadPart = timestamp.QuadPart - before;
	elapsed.QuadPart *= 1000;
	elapsed.QuadPart /= frequency.QuadPart;

	return elapsed.QuadPart;
}

double lodge_timestamp_elapsed_us(lodge_timestamp_t before)
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER timestamp;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&timestamp);

	LARGE_INTEGER elapsed;
	elapsed.QuadPart = timestamp.QuadPart - before;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= frequency.QuadPart;

	return elapsed.QuadPart;
}

static double get_milliseconds()
{
	return get_seconds() * 1000.0f;
}

#else

#include <sys/time.h>

#if 0
static lodge_timestamp_t get_cycles()
{
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
}
#endif

lodge_timestamp_t lodge_timestamp_get()
{
    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    return start;
}

double lodge_timestamp_elapsed_us(lodge_timestamp_t start)
{
	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
	return (double)delta_us;
}

double lodge_timestamp_elapsed_ms(lodge_timestamp_t start)
{
	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	uint64_t delta_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000;
	return (double)delta_ms;
}

double lodge_timestamp_elapsed_s(lodge_timestamp_t start)
{
	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	uint64_t delta_s = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000;
	return (double)delta_s;
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
