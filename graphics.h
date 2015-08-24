#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "math4.h"

struct sprite {
    vec4    pos;
    vec4    scale;
    vec4    color;
    float   rotation;
};

#define UNIFORM_TRANSFORM_NAME  "transform"
#define UNIFORM_PROJECTION_NAME "projection"
#define UNIFORM_COLOR_NAME      "color"

struct shader {
    GLuint  program;
    GLint   uniform_transform;
    GLint   uniform_projection;
    GLint   uniform_color;
    GLint   *uniforms;
};

struct graphics {
    int             frames;                     /* Number of frames drawn since last_frame_report. */
    double          last_frame_report;          /* When frames were last summed up. */
    GLuint          vbo_rect;                   /* Vertex Buffer Object. */
    GLuint          vao_rect;                   /* Vertex Array Object. */
    mat4            projection;                 /* Projection matrix. */
    mat4            translate;                  /* Global translation matrix. */
    mat4            rotate;                     /* Global rotation matrix. */
    mat4            scale;                      /* Global scale matrix. */
    struct shader   shader;                     /* Shader program information. */
};

void graphics_init(struct graphics *g, int view_width, int view_height,
        const char **vertex_shader_src, const char **fragment_shader_src,
        const char **uniform_names, int uniforms_count);
void graphics_free(struct graphics *g);
void graphics_count_frame(struct graphics *g);

void shader_init(struct shader *s, const char **vertex_shader_src,
        const char **fragment_shader_src, const char **uniform_names,
        int uniforms_count);
void shader_free(struct shader *s);
void shader_program_log(GLuint program, const char *name);
void shader_log(GLuint shader, const char *name);

void sprite_render(struct sprite *sprite, struct graphics *g);

#endif
