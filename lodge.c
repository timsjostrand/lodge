#include "vfs.h"
#include "core.h"
#include "assets.h"

struct lodge_ret lodge_ret_make_success()
{
	struct lodge_ret tmp = {
		.success = 1,
		.message = strview_static(""),
	};
	return tmp;
}

struct lodge_ret lodge_ret_make_error(strview_t message)
{
	struct lodge_ret tmp = {
		.success = 0,
		.message = message,
	};
	return tmp;
}