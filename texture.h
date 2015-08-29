#ifndef _TEXTURE_H
#define _TEXTURE_H

int  image_load(uint8_t **data, int *width, int *height, const char *path);
void image_free(uint8_t *data);

int  texture_load_data(GLuint *tex, const uint8_t *data,
        const int width, const int height);
int  texture_load(GLuint *tex, const char *path);
void texture_white(GLuint *tex);
void texture_free(const GLuint tex);

#endif
