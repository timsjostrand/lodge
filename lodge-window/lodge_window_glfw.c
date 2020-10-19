#define LODGE_GLFW // TODO: Set from cmake

#ifdef LODGE_GLFW

#include "lodge_window.h"
#include "lodge_plugins.h"
#include "lodge_renderer.h"
#include "lodge_keys.h"
#include "log.h"

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#define lodge_window_debug(...) debugf("lodge_window", __VA_ARGS__)
#define lodge_window_error(...) errorf("lodge_window", __VA_ARGS__)

#define WINDOW_TITLE_MAX 256

struct input {
	int									keys[LODGE_KEY_LAST];		/* Key status of current frame. */
	int									last_keys[LODGE_KEY_LAST];	/* Key status of last frame. */
};

struct glfw_window
{
	char								title[WINDOW_TITLE_MAX];

	GLFWwindow*							window;
	int									window_mode;

	int									windowed_width;
	int									windowed_height;
	int									windowed_pos_x;
	int									windowed_pos_y;

	lodge_window_mousebutton_callback_t callback_mousebutton;
	void								*callback_mousebutton_userdata;

	lodge_window_scroll_callback_t		callback_scroll;
	void								*callback_scroll_userdata;

	lodge_window_input_callback_t		callback_input;
	void								*callback_input_userdata;

	lodge_window_input_char_callback_t	callback_char;
	void								*callback_char_userdata;

	lodge_window_resize_callback_t		callback_resize;
	void								*callback_resize_userdata;

	struct input						input;
};

struct glfw_window*	cast_handle(lodge_window_t window)		{ return (struct glfw_window*)window; }
lodge_window_t		to_handle(struct glfw_window* window)	{ return (lodge_window_t)window; }

static void glfw_error_callback(int error_code, const char *msg)
{
	lodge_window_error("GLFW Error %d: %s\n", error_code, msg);
}

static void glfw_mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	if(glfw_window->callback_mousebutton) {
		glfw_window->callback_mousebutton(to_handle(glfw_window), button, action, mods, glfw_window->callback_mousebutton_userdata);
	}
}

static void glfw_scroll_callback(GLFWwindow *window, double x, double y)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	if(glfw_window->callback_scroll) {
		glfw_window->callback_scroll(to_handle(glfw_window), x, y, glfw_window->callback_mousebutton_userdata);
	}
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	struct glfw_window *glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);

	/* Sanity check */
	if (key < 0 || key >= LODGE_KEY_LAST) {
		debugf("Input", "Invalid key: %d (scancode=%d)\n", key, scancode);
		return;
	}

	/* Only care about 'up'/'down', regard 'repeat' as 'down'. */
	glfw_window->input.keys[key] = !(action == LODGE_RELEASE);

	if(glfw_window->callback_input) {
		glfw_window->callback_input(to_handle(glfw_window), key, scancode, action, mods, glfw_window->callback_input_userdata);
	}
}

static void glfw_char_callback(GLFWwindow *window, unsigned int key, int mods)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	if(glfw_window->callback_char) {
		glfw_window->callback_char(to_handle(glfw_window), key, mods, glfw_window->callback_char_userdata);
	}
}

static void glfw_resize_callback(GLFWwindow *window, int width, int height)
{
	struct glfw_window* glfw_window = (struct glfw_window*)glfwGetWindowUserPointer(window);
	if(glfw_window->callback_resize) {
		glfw_window->callback_resize(to_handle(glfw_window), width, height, glfw_window->callback_resize_userdata);
	}
}

static struct lodge_ret lodge_windows_initialize(struct lodge_windows *windows, struct lodge_plugins *plugins)
{
	windows->library = strview_static("glfw");

	int ret = glfwInit();
	if (ret)
	{
		glfwSetErrorCallback(&glfw_error_callback);
		return lodge_success();
	}
	else
	{
		return lodge_error("Could not initialize GLFW");
	}
}

static void lodge_windows_shutdown(struct lodge_windows *windows)
{
	glfwTerminate();
}

static void lodge_windows_update(struct lodge_windows *windows, float dt)
{
	for(int i = 0; i < windows->windows_count; i++) {
		lodge_window_update(windows->windows[i]);
	}
}

static void lodge_windows_render(struct lodge_windows *windows)
{
	for(int i = 0; i < windows->windows_count; i++) {
		lodge_window_render(windows->windows[i]);
	}
}

struct lodge_plugin lodge_plugin_windows()
{
	struct lodge_plugin plugin = {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_windows),
		.name = strview_static("windows"),
		.init = &lodge_windows_initialize,
		.free = &lodge_windows_shutdown,
		.update = &lodge_windows_update,
		.render = &lodge_windows_render,
	};
	return plugin;
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
	glfwSetScrollCallback(window, &glfw_scroll_callback);
	glfwSetFramebufferSizeCallback(window, &glfw_resize_callback);

	return window;
}

lodge_window_t lodge_window_new(struct lodge_windows *windows, const char *title, int window_width, int window_height, int window_mode)
{
	struct glfw_window* glfw_window = (struct glfw_window*)malloc(sizeof(struct glfw_window));

	glfw_window->window = glfw_window_create(title, window_width, window_height, window_mode, NULL);
	glfwSetWindowUserPointer(glfw_window->window, (void*)glfw_window);

	glfwMakeContextCurrent(glfw_window->window);

	glfw_window->window_mode = window_mode;
	glfw_window->callback_mousebutton = NULL;
	glfw_window->callback_mousebutton_userdata = NULL;
	glfw_window->callback_scroll = NULL;
	glfw_window->callback_scroll_userdata = NULL;
	glfw_window->callback_input = NULL;
	glfw_window->callback_input_userdata = NULL;
	glfw_window->callback_char = NULL;
	glfw_window->callback_char_userdata = NULL;
	glfw_window->callback_resize = NULL;
	glfw_window->callback_resize_userdata = NULL;

	memset(&glfw_window->input, 0, sizeof(struct input));

	strcpy(glfw_window->title, title);

	lodge_window_t window = to_handle(glfw_window);

	windows->windows[windows->windows_count++] = window;

	return window;
}

void lodge_window_free(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwSetWindowShouldClose(glfw_window->window, 1);
	free(glfw_window);
}

struct lodge_ret lodge_window_set_renderer(lodge_window_t window, struct lodge_renderer *renderer)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwMakeContextCurrent(glfw_window->window);
	struct lodge_ret attach_ret = lodge_renderer_attach(renderer);
	if(!attach_ret.success) {
		ASSERT_FAIL("Failed to attach renderer");
		errorf("Window", "Failed to attach renderer: %s\n", attach_ret.message.s);
		return attach_ret;
	}
	return lodge_success();
}

void lodge_window_update(lodge_window_t window)
{
	struct glfw_window *glfw_window = cast_handle(window);
	// Remember key state
	memcpy(glfw_window->input.last_keys, glfw_window->input.keys, sizeof(glfw_window->input.last_keys));
}

void lodge_window_render(lodge_window_t window)
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

void lodge_window_set_mousebutton_callback(lodge_window_t window, lodge_window_mousebutton_callback_t callback, void *userdata)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_mousebutton = callback;
	glfw_window->callback_mousebutton_userdata = userdata;
}

void lodge_window_set_scroll_callback(lodge_window_t window, lodge_window_scroll_callback_t callback, void *userdata)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_scroll = callback;
	glfw_window->callback_scroll_userdata = userdata;
}

void lodge_window_set_input_callback(lodge_window_t window, lodge_window_input_callback_t callback, void *userdata)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_input = callback;
	glfw_window->callback_input_userdata = userdata;
}

void lodge_window_set_input_char_callback(lodge_window_t window, lodge_window_input_char_callback_t callback, void *userdata)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_char = callback;
	glfw_window->callback_char_userdata = userdata;
}

void lodge_window_set_resize_callback(lodge_window_t window, lodge_window_resize_callback_t callback, void *userdata)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfw_window->callback_resize = callback;
	glfw_window->callback_resize_userdata = userdata;
}

void lodge_window_set_cursor_mode(lodge_window_t window, int value)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwSetInputMode(glfw_window->window, GLFW_CURSOR, value);
}

void lodge_window_set_should_close(lodge_window_t window, int value)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwSetWindowShouldClose(glfw_window->window, value);
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

int lodge_window_get_mouse_button(lodge_window_t window, int mouse_button)
{
	struct glfw_window* glfw_window = cast_handle(window);
	return glfwGetMouseButton(glfw_window->window, mouse_button);
}

int lodge_window_is_focused(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);
	return glfwGetWindowAttrib(glfw_window->window, GLFW_FOCUSED);
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

void lodge_window_set_vsync_enabled(lodge_window_t window, int vsync)
{
	struct glfw_window* glfw_window = cast_handle(window);
	glfwMakeContextCurrent(glfw_window->window);
	glfwSwapInterval(vsync ? 1 : 0);
}

int lodge_window_key_down(lodge_window_t window, int key)
{
	struct glfw_window *glfw_window = cast_handle(window);
	return glfw_window->input.keys[key];
}

int lodge_window_key_pressed(lodge_window_t window, int key)
{
	struct glfw_window *glfw_window = cast_handle(window);
	//return (glfw_window->input.keys[key] && !glfw_window->input.last_keys[key]);
	return glfwGetKey(glfw_window->window, key);
}

int lodge_window_key_released(lodge_window_t window, int key)
{
	struct glfw_window *glfw_window = cast_handle(window);
	return (!glfw_window->input.keys[key] && glfw_window->input.last_keys[key]);
}

strview_t lodge_window_get_clipboard(lodge_window_t window)
{
	struct glfw_window* glfw_window = cast_handle(window);

	const char *str = glfwGetClipboardString(glfw_window->window);
	const size_t str_len = strlen(str);
	return strview_make(str, str_len);
}

void lodge_window_set_clipboard(lodge_window_t window, strview_t str)
{
	struct glfw_window* glfw_window = cast_handle(window);

	// Make sure str is null terminated
	char *tmp = malloc(str.length + 1);
	memcpy(tmp, str.s, str.length);
	tmp[str.length] = '\0';
	glfwSetClipboardString(glfw_window->window, tmp);
	free(tmp);
}

#endif
