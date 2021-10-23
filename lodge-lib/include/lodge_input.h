#ifndef _LODGE_INPUT_H
#define _LODGE_INPUT_H

#include "math4.h"

#include <stdbool.h>

struct lodge_input;

void	lodge_input_new_inplace(struct lodge_input *input);
void	lodge_input_free_inplace(struct lodge_input *input);
size_t	lodge_input_sizeof();

bool	lodge_input_is_key_down(const struct lodge_input *input, uint32_t key);
bool	lodge_input_is_mouse_button_down(const struct lodge_input *input, uint32_t mouse_button);
bool	lodge_input_is_mouse_button_position_valid(const struct lodge_input *input);
vec2	lodge_input_get_mouse_position(const struct lodge_input *input);

void	lodge_input_set_key_down(struct lodge_input *input, uint32_t key, bool down);
void	lodge_input_set_mouse_button_down(struct lodge_input *input, uint32_t mouse_button, bool down);
void	lodge_input_set_mouse_position(struct lodge_input *input, vec2 position);
void	lodge_input_invalidate_mouse_position(struct lodge_input *input);

#endif