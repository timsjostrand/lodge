/**
 * Collision detection.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <math.h>

#include "collide.h"

int collide_rect(const struct rect *a, const struct rect *b)
{
	return collide_rectf(xywh(a->pos, a->size), xywh(b->pos, b->size));
}

int collide_circle(const struct circle *a, const struct circle *b)
{
	return collide_circlef(a->pos[0], a->pos[1], a->r,
			b->pos[0], b->pos[1], b->r);
}

int collide_rectf(const float x1, const float y1, const float w1, const float h1,
		const float x2, const float y2, const float w2, const float h2)
{
#if 1
	return (fabs(x1 - x2)*2 < (w1 + w2)) && (fabs(y1 - y2)*2 < (h1 + h2));
#else
	return !(x2-w2/2.0f > x1+w1/2.0f
			|| x2+w2/2.0f < x1-w1/2.0f
			|| y2+h2/2.0f < y1-h1/2.0f
			|| y2-h2/2.0f > y1+h1/2.0f);
#endif
}

int collide_circlef(const float x1, const float y1, const float r1, const float x2, const float y2, const float r2)
{
	return (x2-x1)*(x2-x1) + (y1-y2)*(y1-y2) <= (r1+r2)*(r1+r2);
}
