#define LODGE_GLFW // TODO: Set from cmake

#ifdef LODGE_GLFW

#include "lodge_window.h"
#include "log.h"

#include <GLFW/glfw3.h>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#define lodge_window_debug(...) debugf("lodge_window", __VA_ARGS__)
#define lodge_window_error(...) errorf("lodge_window", __VA_ARGS__)

#define WINDOW_TITLE_MAX 256

struct glfw_window
{
	char								title[WINDOW_TITLE_MAX];

	GLFWwindow*							window;
	int									window_mode;

	int									windowed_width;
	int									windowed_height;
	int									windowed_pos_x;
	int									windowed_pos_y;

	lodge_window_create_callback_t		callback_create;
	lodge_window_mousebutton_callback_t callback_mousebutton;
	lodge_window_input_callback_t		callback_input;
	lodge_window_input_char_callback_t	callback_char;
	lodge_window_resize_callback_t		callback_resize;

	void*								userdata;
};

struct glfw_window*	cast_handle(lodge_window_t window)		{ return (struct glfw_window*)window; }
lodge_window_t		to_handle(struct glfw_window* window)	{ return (lodge_window_t)window; }

static void glfw_error_callback(int error_code, const char *msg)
{
	lodge_window_error("GLFW Error %d: %s\n", error_code, msg);
}

void glfw_mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	glfw_window->callback_mousebutton(to_handle(glfw_window), button, action, mods);
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	glfw_window->callback_input(to_handle(glfw_window), key, scancode, action, mods);
}

void glfw_char_callback(GLFWwindow *window, unsigned int key, int mods)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	glfw_window->callback_char(to_handle(glfw_window), key, mods);
}

void glfw_resize_callback(GLFWwindow *window, int width, int height)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	glfw_window->callback_resize(to_handle(glfw_window), width, height);
}

void lodge_window_initialize()
{
	if (glfwInit()) 
	{
		glfwSetErrorCallback(&glfw_error_callback);
	}
	else
	{
		printf("Could not initialize glfw (%d)\n"); // TODO: Where to log this?
	}
}

void lodge_window_shutdown()
{
	glfwTerminate();
}

GLFWwindow* glfw_window_create(const char *title, int window_width, int window_height, int window_mode, GLFWwindow* window_old)
{
	GLFWwindow* window;

#ifdef EMSCRIPTEN
	window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);
	if (!window)
	{
		return 0;
	}
#else

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (window_mode == LODGE_WINDOW_MODE_WINDOWED)
	{
		window = glfwCreateWindow(window_width, window_height, title, NULL, window_old);
	}
	else if (window_mode == LODGE_WINDOW_MODE_BORDERLESS)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
		glfwWindowHint(GLFW_FLOATING, GL_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);

		window = glfwCreateWindow(video_mode->width, video_mode->height, title, NULL, window_old);
	}
	else
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);
		window = glfwCreateWindow(video_mode->width, video_mode->height, title, monitor, window_old);
	}

	if (!window)
	{
		return 0;
	}
#endif

	glfwSetKeyCallback(window, &glfw_key_callback);
	glfwSetCharModsCallback(window, &glfw_char_callback);
	glfwSetMouseButtonCallback(window, &glfw_mousebutton_callback);
	glfwSetFramebufferSizeCallback(window, &glfw_resize_callback);

	return window;
}

lodge_window_t lodge_window_create(const char *title, int window_width, int window_height, int window_mode, lodge_window_create_callback_t create_callback)
{
	struct glfw_window* glfw_window = (struct glfw_window*)malloc(sizeof(struct glfw_window));

	glfw_window->window = glfw_window_create(title, window_width, window_height, window_mode, 0);
	glfwMakeContextCurrent(glfw_window->window);
	glfwSetWindowUserPointer(glfw_window->window, (void*)glfw_window);

	glfw_window->window_mode = window_mode;
	glfw_window->callback_mousebutton = 0;
	glfw_window->callback_input = 0;
	glfw_window->callback_char = 0;
	glfw_window->callback_resize;
	glfw_window->callback_create = create_callback;
	glfw_window->userdata = 0;

	strcpy(glfw_window->title, title);

	lodge_window_t window = to_handle(glfw_window);

	if (glfw_window->callback_create)
		glfw_window->callback_create(window);

	return window;
}

void lodge_window_destroy(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwSetWindowShouldClose(glfw_window->window, 1);
}

void lodge_window_update(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwPollEvents();
	glfwSwapBuffers(glfw_window->window);
}

void lodge_window_set_mode(lodge_window_t window, int window_mode)
{
	struct glfw_window* glfw_window = cast_handle(window);

	glfw_window->window_mode = window_mode;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);

	if (glfw_window->window_mode == LODGE_WINDOW_MODE_FULLSCREEN)
	{
		glfwGetWindowPos(glfw_window->window, &glfw_window->windowed_pos_x, &glfw_window->windowed_pos_y);
		glfwGetWindowSize(glfw_window->window, &glfw_window->windowed_width, &glfw_window->windowed_height);

		glfwSetWindowMonitor(glfw_window->window, monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
	}
	else if (glfw_window->window_mode == LODGE_WINDOW_MODE_WINDOWED)
	{
		glfwSetWindowMonitor(	glfw_window->window, 
								0, 
								glfw_window->windowed_pos_x, 
								glfw_window->windowed_pos_y, 
								glfw_window->windowed_width, 
								glfw_window->windowed_height, 
								0);
	}
	else if (glfw_window->window_mode == LODGE_WINDOW_MODE_BORDERLESS)
	{
		// TODO: We cannot switch to borderless in runtime because glfw only supports window hints for glfwCreateWindow
	}
}

int lodge_window_get_mode(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	return glfw_window->window_mode;
}

void lodge_window_toggle_fullscreen(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	lodge_window_set_mode(window, glfw_window->window_mode ? LODGE_WINDOW_MODE_FULLSCREEN : LODGE_WINDOW_MODE_WINDOWED);
}

int lodge_window_is_open(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	return !glfwWindowShouldClose(glfw_window->window);
}

void lodge_window_set_mousebutton_callback(lodge_window_t window, lodge_window_mousebutton_callback_t callback)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_mousebutton = callback;
}

void lodge_window_set_input_callback(lodge_window_t window, lodge_window_input_callback_t callback)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_input = callback;
}

void lodge_window_set_input_char_callback(lodge_window_t window, lodge_window_input_char_callback_t callback)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_char = callback;
}

void lodge_window_set_resize_callback(lodge_window_t window, lodge_window_resize_callback_t callback)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_resize = callback;
}

void lodge_window_set_userdata(lodge_window_t window, void* userdata)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->userdata = userdata;
}

void* lodge_window_get_userdata(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	return glfw_window->userdata;
}

void lodge_window_get_size(lodge_window_t window, int* width, int* height)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwGetWindowSize(glfw_window->window, width, height);
}

void lodge_window_get_cursor(lodge_window_t window, float* x, float* y)
{
	struct glfw_window* glfw_window = cast_handle(window);

	double tmp_x, tmp_y;
	glfwGetCursorPos(glfw_window->window, &tmp_x, &tmp_y);

	*x = (float)tmp_x;
	*y = (float)tmp_y;
}

double lodge_window_get_time()
{
	return glfwGetTime() * 1000.0;
}

void lodge_window_get_screen_size(int* width, int* height)
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);

	*width = video_mode->width;
	*height = video_mode->height;
}

#endif
