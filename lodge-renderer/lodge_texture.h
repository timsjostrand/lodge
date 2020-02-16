#ifndef _LODGE_TEXTURE_H
#define _LODGE_TEXTURE_H

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

//texture_t			lodge_texture_make();
lodge_texture_t		lodge_texture_make_rgba(int width, int height);
lodge_texture_t		lodge_texture_make_depth(int width, int height);

void				lodge_texture_reset(lodge_texture_t *texture);

#endif