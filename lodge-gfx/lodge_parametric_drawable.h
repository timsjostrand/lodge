#pragma once

#include "math4.h"

struct lodge_drawable;
typedef struct lodge_drawable* lodge_drawable_t;

lodge_drawable_t lodge_drawable_make_plane_subdivided(vec2 origin, vec2 size, uint32_t divisions_x, uint32_t divisions_y);