#include "log.h"

#define core_argv_error(...) errorf("Argv", __VA_ARGS__)

#define CORE_ARGV_NAME_MAX 256
#define CORE_ARGV_VALUE_MAX 256

struct core_argv {
	int		window_mode;
	char	mount[CORE_ARGV_VALUE_MAX];
	char	game[CORE_ARGV_VALUE_MAX];
};

int core_argv_parse(struct core_argv *dst, int argc, char **argv);