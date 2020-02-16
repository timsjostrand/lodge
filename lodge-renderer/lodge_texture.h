#ifndef _LODGE_TEXTURE_H
#define _LODGE_TEXTURE_H

struct lodge_texture;
typedef struct lodge_texture* texture_t;

//texture_t		lodge_texture_make();
texture_t		lodge_texture_make_rgba(int width, int height);
texture_t		lodge_texture_make_depth(int width, int height);

void			lodge_texture_reset(texture_t *texture);

#endif