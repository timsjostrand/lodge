#pragma once

//
// Allows storing and calling object function pointers with arbitrary arguments,
// given a func interface such as:
// 
//     struct
//     {
//         <ret>	(*func)(void *object, ...)
//         void		*object;
//     } my_func;
// 
// Usage:
// 
//     <ret> value = lodge_func_call(my_func, ...);
// 
// Example:
// 
//     See `struct lodge_scene_funcs`.
//

#define lodge_bound_func_is_set(f) \
	(f.func != NULL && f.object != NULL)

#define lodge_bound_func_call(f, ...) \
	f.func(f.object, __VA_ARGS__)

#define lodge_bound_func_set(f, OBJECT, FUNC) \
	f.object = (void*)OBJECT; f.func = FUNC

#define lodge_bound_func_reset(f) \
	f.object = NULL; f.func = NULL
