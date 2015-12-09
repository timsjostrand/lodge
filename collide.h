#ifndef _COLLIDE_H
#define _COLLIDE_H

#include "math4.h"
#include "geometry.h"

#define xywh(pos, size) pos[0], pos[1], size[0], size[1]

int collide_rect(const struct rect *a, const struct rect *b);
int collide_circle(const struct circle *a, const struct circle *b);

int collide_rectf(const float x1, const float y1, const float w1, const float h1,
		const float x2, const float y2, const float w2, const float h2);
int collide_circlef(const float x1, const float y1, const float r1,
		const float x2, const float y2, const float r2);

#endif
