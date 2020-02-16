#include "lodge_platform.h"

#include "log.h"
#include "lodge_window.h" // for lodge_get_time()

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

double lodge_get_time()
{
	// NOTE(TS): should be custom implementation, but this works for now
	return lodge_window_get_time();
}

void* lodge_lib_load(const char *filename)
{
	void *library = 0;

#ifdef _WIN32
	library = (void *)LoadLibrary(filename);
#else
	library = dlopen(filename, 2);
#endif

	if(!library)
	{
		errorf("Platform", "Could not load library %s\n", filename);
	}

	return library;
}

void* lodge_lib_load_copy(const char *filename, size_t size, void *data)
{
	char name[256];

	char *tmp = strrchr(filename, (int)'/');
	if(tmp) {
		tmp++;
		int namebegin = strlen(filename) - strlen(tmp);
		strcpy(name, filename);
		strcpy(&name[namebegin], "runtime_");
		strcpy(&name[namebegin + 8], tmp);
	} else {
		strcpy(name, "runtime_");
		strcpy(name, filename);
	}

	FILE *fp;
	fp = fopen(name, "wb+");
	fwrite(data, sizeof(char), size, fp);
	fclose(fp);

	return load_shared_library(name);
}

void* lodge_lib_get_symbol(void *library, const char *symbol_name)
{
	void *symbol = 0;

#ifdef _WIN32
	symbol = (void *)GetProcAddress((HINSTANCE)library, symbol_name);
#else
	symbol = dlsym(library, symbol_name);
#endif

	if(!symbol) {
		errorf("Platform", "Could not load function %s\n", symbol_name);
	}

	return function;
}

int lodge_lib_free(void *library)
{
#ifdef _WIN32
	return FreeLibrary((HINSTANCE)library);
#else
	return dlclose(library);
#endif
}
