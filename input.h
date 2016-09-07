#ifndef _INPUT_H
#define _INPUT_H

#include "lodge_window.h"

#define LODGE_RELEASE                0
#define LODGE_PRESS                  1
#define LODGE_REPEAT                 2

#define LODGE_KEY_UNKNOWN            -1
#define LODGE_KEY_SPACE              32
#define LODGE_KEY_APOSTROPHE         39
#define LODGE_KEY_COMMA              44
#define LODGE_KEY_MINUS              45
#define LODGE_KEY_PERIOD             46
#define LODGE_KEY_SLASH              47
#define LODGE_KEY_0                  48
#define LODGE_KEY_1                  49
#define LODGE_KEY_2                  50
#define LODGE_KEY_3                  51
#define LODGE_KEY_4                  52
#define LODGE_KEY_5                  53
#define LODGE_KEY_6                  54
#define LODGE_KEY_7                  55
#define LODGE_KEY_8                  56
#define LODGE_KEY_9                  57
#define LODGE_KEY_SEMICOLON          59
#define LODGE_KEY_EQUAL              61
#define LODGE_KEY_A                  65
#define LODGE_KEY_B                  66
#define LODGE_KEY_C                  67
#define LODGE_KEY_D                  68
#define LODGE_KEY_E                  69
#define LODGE_KEY_F                  70
#define LODGE_KEY_G                  71
#define LODGE_KEY_H                  72
#define LODGE_KEY_I                  73
#define LODGE_KEY_J                  74
#define LODGE_KEY_K                  75
#define LODGE_KEY_L                  76
#define LODGE_KEY_M                  77
#define LODGE_KEY_N                  78
#define LODGE_KEY_O                  79
#define LODGE_KEY_P                  80
#define LODGE_KEY_Q                  81
#define LODGE_KEY_R                  82
#define LODGE_KEY_S                  83
#define LODGE_KEY_T                  84
#define LODGE_KEY_U                  85
#define LODGE_KEY_V                  86
#define LODGE_KEY_W                  87
#define LODGE_KEY_X                  88
#define LODGE_KEY_Y                  89
#define LODGE_KEY_Z                  90
#define LODGE_KEY_LEFT_BRACKET       91
#define LODGE_KEY_BACKSLASH          92
#define LODGE_KEY_RIGHT_BRACKET      93
#define LODGE_KEY_GRAVE_ACCENT       96
#define LODGE_KEY_WORLD_1            161
#define LODGE_KEY_WORLD_2            162 
#define LODGE_KEY_ESCAPE             256
#define LODGE_KEY_ENTER              257
#define LODGE_KEY_TAB                258
#define LODGE_KEY_BACKSPACE          259
#define LODGE_KEY_INSERT             260
#define LODGE_KEY_DELETE             261
#define LODGE_KEY_RIGHT              262
#define LODGE_KEY_LEFT               263
#define LODGE_KEY_DOWN               264
#define LODGE_KEY_UP                 265
#define LODGE_KEY_PAGE_UP            266
#define LODGE_KEY_PAGE_DOWN          267
#define LODGE_KEY_HOME               268
#define LODGE_KEY_END                269
#define LODGE_KEY_CAPS_LOCK          280
#define LODGE_KEY_SCROLL_LOCK        281
#define LODGE_KEY_NUM_LOCK           282
#define LODGE_KEY_PRINT_SCREEN       283
#define LODGE_KEY_PAUSE              284
#define LODGE_KEY_F1                 290
#define LODGE_KEY_F2                 291
#define LODGE_KEY_F3                 292
#define LODGE_KEY_F4                 293
#define LODGE_KEY_F5                 294
#define LODGE_KEY_F6                 295
#define LODGE_KEY_F7                 296
#define LODGE_KEY_F8                 297
#define LODGE_KEY_F9                 298
#define LODGE_KEY_F10                299
#define LODGE_KEY_F11                300
#define LODGE_KEY_F12                301
#define LODGE_KEY_F13                302
#define LODGE_KEY_F14                303
#define LODGE_KEY_F15                304
#define LODGE_KEY_F16                305
#define LODGE_KEY_F17                306
#define LODGE_KEY_F18                307
#define LODGE_KEY_F19                308
#define LODGE_KEY_F20                309
#define LODGE_KEY_F21                310
#define LODGE_KEY_F22                311
#define LODGE_KEY_F23                312
#define LODGE_KEY_F24                313
#define LODGE_KEY_F25                314
#define LODGE_KEY_KP_0               320
#define LODGE_KEY_KP_1               321
#define LODGE_KEY_KP_2               322
#define LODGE_KEY_KP_3               323
#define LODGE_KEY_KP_4               324
#define LODGE_KEY_KP_5               325
#define LODGE_KEY_KP_6               326
#define LODGE_KEY_KP_7               327
#define LODGE_KEY_KP_8               328
#define LODGE_KEY_KP_9               329
#define LODGE_KEY_KP_DECIMAL         330
#define LODGE_KEY_KP_DIVIDE          331
#define LODGE_KEY_KP_MULTIPLY        332
#define LODGE_KEY_KP_SUBTRACT        333
#define LODGE_KEY_KP_ADD             334
#define LODGE_KEY_KP_ENTER           335
#define LODGE_KEY_KP_EQUAL           336
#define LODGE_KEY_LEFT_SHIFT         340
#define LODGE_KEY_LEFT_CONTROL       341
#define LODGE_KEY_LEFT_ALT           342
#define LODGE_KEY_LEFT_SUPER         343
#define LODGE_KEY_RIGHT_SHIFT        344
#define LODGE_KEY_RIGHT_CONTROL      345
#define LODGE_KEY_RIGHT_ALT          346
#define LODGE_KEY_RIGHT_SUPER        347
#define LODGE_KEY_MENU               348
#define LODGE_KEY_LAST               LODGE_KEY_MENU

#define LODGE_MOD_SHIFT           0x0001
#define LODGE_MOD_CONTROL         0x0002
#define LODGE_MOD_ALT             0x0004
#define LODGE_MOD_SUPER           0x0008

#define LODGE_MOUSE_BUTTON_1         0
#define LODGE_MOUSE_BUTTON_2         1
#define LODGE_MOUSE_BUTTON_3         2
#define LODGE_MOUSE_BUTTON_4         3
#define LODGE_MOUSE_BUTTON_5         4
#define LODGE_MOUSE_BUTTON_6         5
#define LODGE_MOUSE_BUTTON_7         6
#define LODGE_MOUSE_BUTTON_8         7
#define LODGE_MOUSE_BUTTON_LAST      LODGE_MOUSE_BUTTON_8
#define LODGE_MOUSE_BUTTON_LEFT      LODGE_MOUSE_BUTTON_1
#define LODGE_MOUSE_BUTTON_RIGHT     LODGE_MOUSE_BUTTON_2
#define LODGE_MOUSE_BUTTON_MIDDLE    LODGE_MOUSE_BUTTON_3

#define LODGE_JOYSTICK_1             0
#define LODGE_JOYSTICK_2             1
#define LODGE_JOYSTICK_3             2
#define LODGE_JOYSTICK_4             3
#define LODGE_JOYSTICK_5             4
#define LODGE_JOYSTICK_6             5
#define LODGE_JOYSTICK_7             6
#define LODGE_JOYSTICK_8             7
#define LODGE_JOYSTICK_9             8
#define LODGE_JOYSTICK_10            9
#define LODGE_JOYSTICK_11            10
#define LODGE_JOYSTICK_12            11
#define LODGE_JOYSTICK_13            12
#define LODGE_JOYSTICK_14            13
#define LODGE_JOYSTICK_15            14
#define LODGE_JOYSTICK_16            15
#define LODGE_JOYSTICK_LAST          LODGE_JOYSTICK_16

struct input {
	int										enabled;
	lodge_window_input_callback_t			callback;
	lodge_window_mousebutton_callback_t		mousebutton_callback;
	lodge_window_input_char_callback_t		char_callback;
	int										keys[LODGE_KEY_LAST];		/* Key status of current frame. */
	int										last_keys[LODGE_KEY_LAST];	/* Key status of last frame. */
};

int		input_init(struct input *input, lodge_window_t window,
					lodge_window_input_callback_t key_callback, 
					lodge_window_input_char_callback_t char_callback, 
					lodge_window_mousebutton_callback_t mousebutton_callback);

void	input_think(struct input *input, float delta_time);

int		key_down(int key);
int		key_pressed(int key);
int		key_released(int key);

struct input *input_global;

#endif
