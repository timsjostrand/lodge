#pragma once

#include "strview.h"

enum lodge_log_level
{
	LODGE_LOG_LEVEL_DEBUG = 0,
	LODGE_LOG_LEVEL_INFO,
	LODGE_LOG_LEVEL_WARNING,
	LODGE_LOG_LEVEL_ERROR,
	LODGE_LOG_LEVEL_MAX,
};

typedef void	(*lodge_log_func_t)(enum lodge_log_level level, strview_t src, strview_t message, void *userdata);

void			lodge_log(enum lodge_log_level level, strview_t src, strview_t message);
void			lodge_logf(enum lodge_log_level level, strview_t src, strview_t fmt, ...);

void			lodge_log_add_func(lodge_log_func_t func, void *userdata);
void			lodge_log_remove_func(lodge_log_func_t func, void *userdata);

#ifdef _DEBUG
	#define debugf(module, fmt, ...) \
		lodge_logf(LODGE_LOG_LEVEL_DEBUG, strview(module), strview(fmt), __VA_ARGS__)
#else
	#define debugf(...) do {} while (0)
#endif

#define errorf(module, fmt, ...) \
	lodge_logf(LODGE_LOG_LEVEL_ERROR, strview(module), strview(fmt), __VA_ARGS__)

#define warnf(module, fmt, ...) \
	lodge_logf(LODGE_LOG_LEVEL_WARNING, strview(module), strview(fmt), __VA_ARGS__)