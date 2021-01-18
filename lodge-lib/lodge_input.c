#include "lodge_input.h"

#include "lodge_keys.h"
#include "lodge_assert.h"

#include <string.h>
#include <float.h>

struct lodge_input
{
	vec2	mouse_pos;
	bool	mouse_down[LODGE_MOUSE_BUTTON_LAST];
	bool	key_down[LODGE_KEY_LAST];
	bool	gamepad_down[LODGE_JOYSTICK_LAST];
};

void lodge_input_new_inplace(struct lodge_input *input)
{
	memset(input, 0, sizeof(struct lodge_input));
}

void lodge_input_free_inplace(struct lodge_input *input)
{
}

size_t lodge_input_sizeof()
{
	return sizeof(struct lodge_input);
}

bool lodge_input_is_key_down(const struct lodge_input *input, uint32_t key)
{
	ASSERT(input && key <= LODGE_KEY_LAST);
	return input ? input->key_down[key] : false;
}

bool lodge_input_is_mouse_button_down(const struct lodge_input *input, uint32_t mouse_button)
{
	ASSERT(input && mouse_button <= LODGE_MOUSE_BUTTON_LAST);
	return input ? input->mouse_down[mouse_button] : false;
}

vec2 lodge_input_get_mouse_position(const struct lodge_input *input)
{
	ASSERT(input);
	return input ? input->mouse_pos : vec2_zero();
}

bool lodge_input_is_mouse_button_position_valid(const struct lodge_input *input)
{
	return input->mouse_pos.x != -FLT_MAX && input->mouse_pos.y != -FLT_MAX;
}

void lodge_input_set_key_down(struct lodge_input *input, uint32_t key, bool down)
{
	ASSERT(input && key <= LODGE_KEY_LAST);
	if(input && key <= LODGE_KEY_LAST) {
		input->key_down[key] = down;
	}
}

void lodge_input_set_mouse_button_down(struct lodge_input *input, uint32_t mouse_button, bool down)
{
	ASSERT(input && mouse_button <= LODGE_MOUSE_BUTTON_LAST);
	if(input && mouse_button <= LODGE_MOUSE_BUTTON_LAST) {
		input->mouse_down[mouse_button] = down;
	}
}

void lodge_input_set_mouse_position(struct lodge_input *input, vec2 position)
{
	ASSERT(input);
	if(input) {
		input->mouse_pos = position;
	}
}

void lodge_input_invalidate_mouse_position(struct lodge_input *input)
{
	input->mouse_pos = (vec2) { -FLT_MAX, -FLT_MAX };
}
