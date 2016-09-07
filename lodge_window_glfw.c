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

struct glfw_window
{
	GLFWwindow*							window;

	lodge_window_mousebutton_callback_t callback_mousebutton;
	lodge_window_input_callback_t		callback_input;
	lodge_window_input_char_callback_t	callback_char;
	lodge_window_resize_callback_t		callback_resize;

	void*								userdata;
};

struct glfw_window*	cast_handle(lodge_window_t handle)		{ return (struct glfw_window*)handle; }
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

lodge_window_t lodge_window_create(const char *title, int window_width, int window_height, int window_mode)
{
	struct glfw_window* glfw_window = (struct glfw_window*)malloc(sizeof(struct glfw_window));

	glfw_window->window = 0;
	glfw_window->callback_mousebutton = 0;
	glfw_window->callback_input = 0;
	glfw_window->callback_char = 0;
	glfw_window->callback_resize;
	glfw_window->userdata = 0;

#ifdef EMSCRIPTEN
	glfw_window->window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);
	if (!glfw_window->window) 
	{
		return 0;
	}
#else
	glfwDestroyWindow(glfw_window->window);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (window_mode == LODGE_WINDOW_MODE_WINDOWED)
	{
		glfw_window->window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);
	}
	else if (window_mode == LODGE_WINDOW_MODE_BORDERLESS)
	{
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *video_mode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
		glfwWindowHint(GLFW_FLOATING, GL_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);

		glfw_window->window = glfwCreateWindow(video_mode->width, video_mode->height, title, NULL, NULL);
	}
	else
	{
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *video_mode = glfwGetVideoMode(monitor);
		glfw_window->window = glfwCreateWindow(video_mode->width, video_mode->height, title, monitor, NULL);
	}

	if (!glfw_window->window)
	{
		return 0;
	}
#endif

	glfwSetKeyCallback(glfw_window->window, &glfw_key_callback);
	glfwSetCharModsCallback(glfw_window->window, &glfw_char_callback);
	glfwSetMouseButtonCallback(glfw_window->window, &glfw_mousebutton_callback);
	glfwSetWindowUserPointer(glfw_window->window, (void*)glfw_window);
	glfwSetFramebufferSizeCallback(glfw_window->window, &glfw_resize_callback);

	glfwMakeContextCurrent(glfw_window->window);

	return to_handle(glfw_window);
}

void lodge_window_destroy(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwDestroyWindow(glfw_window->window);
}

void lodge_window_update(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwPollEvents();
	glfwSwapBuffers(glfw_window->window);
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

#endif
