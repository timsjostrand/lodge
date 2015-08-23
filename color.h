#ifndef _COLOR_H
#define _COLOR_H

#include "math4.h"

#define rgb(v) xyz(v)
#define rgba(v) xyzw(v)

const vec4 COLOR_RED        = { 1.0f, 0.0f, 0.0f, 1.0f };
const vec4 COLOR_GREEN      = { 0.0f, 1.0f, 0.0f, 1.0f };
const vec4 COLOR_BLUE       = { 0.0f, 0.0f, 1.0f, 1.0f };
const vec4 COLOR_WHITE      = { 1.0f, 1.0f, 1.0f, 1.0f };
const vec4 COLOR_BLACK      = { 0.0f, 0.0f, 0.0f, 1.0f };
const vec4 COLOR_YELLOW     = { 1.0f, 1.0f, 0.0f, 1.0f };
const vec4 COLOR_CYAN       = { 0.0f, 1.0f, 1.0f, 1.0f };
const vec4 COLOR_MAGENTA    = { 1.0f, 0.0f, 1.0f, 1.0f };

#endif
