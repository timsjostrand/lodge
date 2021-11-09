#ifndef _LODGE_TIME_H
#define _LODGE_TIME_H

#ifdef _WIN32
typedef long long lodge_timestamp_t;
#else
#include <time.h>
typedef struct timespec lodge_timestamp_t;
#endif

lodge_timestamp_t	lodge_timestamp_get();
double				lodge_timestamp_elapsed_us(lodge_timestamp_t before);
double				lodge_timestamp_elapsed_ms(lodge_timestamp_t before);
double				lodge_timestamp_elapsed_s(lodge_timestamp_t before);

double				lodge_get_time_ms();
double				lodge_get_time_s();
double				lodge_get_time(); // same as `lodge_get_time_s`, for compat

#endif