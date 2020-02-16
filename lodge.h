#ifndef _LODGE_H
#define _LODGE_H

#include "lodge_platform.h"
#include "math4.h"
#include "str.h"
#include "log.h"

#include <math.h>
#include <string.h>

struct lodge_ret
{
	int			success;
	strview_t	message;
};

struct lodge_ret lodge_ret_make_success();
struct lodge_ret lodge_ret_make_error(strview_t message);

#define lodge_success() lodge_ret_make_success()
#define lodge_error(msg) lodge_ret_make_error(strview_static(msg))

#endif
