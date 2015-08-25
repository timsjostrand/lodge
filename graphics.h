#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "math4.h"

#define GRAPHICS_OK              0
#define GRAPHICS_ERROR          -1
#define GRAPHICS_SHADER_ERROR   -2
#define GRAPHICS_GLFW_ERROR     -3
#define GRAPHICS_GLEW_ERROR     -4

struct sprite {
    int     type;
    vec4    pos;
    vec4    scale;
    vec4    color;
    float   rotation;
};

#define UNIFORM_TRANSFORM_NAME      "transform"
#define UNIFORM_PROJECTION_NAME     "projection"
#define UNIFORM_COLOR_NAME          "color"
#define UNIFORM_SPRITE_TYPE_NAME    "sprite_type"

struct shader {
    GLuint  program;
    GLint   uniform_transform;
    GLint   uniform_projection;
    GLint   uniform_color;
    GLint   uniform_sprite_type;
    GLint   *uniforms;
};

struct graphics;

typedef void (*think_func_t)(struct graphics *g, float delta_time);
typedef void (*render_func_t)(struct graphics *g, float delta_time);

struct graphics {
    GLFWwindow      *window;                    /* The window handle created by GLFW. */
    think_func_t    think;                      /* This function does thinking. */
    render_func_t   render;                     /* This function does rendering. */
    int             frames;                     /* Number of frames drawn since last_frame_report. */
    double          last_frame_report;          /* When frames were last summed up. */
    double          last_frame;                 /* When the last frame was drawn. */
    float           delta_time_factor;          /* Delta-time is multiplied with this factor. */
    GLuint          vbo_rect;                   /* Vertex Buffer Object. */
    GLuint          vao_rect;                   /* Vertex Array Object. */
    mat4            projection;                 /* Projection matrix. */
    mat4            translate;                  /* Global translation matrix. */
    mat4            rotate;                     /* Global rotation matrix. */
    mat4            scale;                      /* Global scale matrix. */
    struct shader   shader;                     /* Shader program information. */
};

int  graphics_init(struct graphics *g, think_func_t think, render_func_t render,
        int view_width, int view_height, int windowed,
        const char **vertex_shader_src, const char **fragment_shader_src,
        const char **uniform_names, int uniforms_count);
void graphics_free(struct graphics *g);
void graphics_count_frame(struct graphics *g);
void graphics_loop();

int  shader_init(struct shader *s, const char **vertex_shader_src,
        const char **fragment_shader_src, const char **uniform_names,
        int uniforms_count);
void shader_free(struct shader *s);
int  shader_program_log(GLuint program, const char *name);
int  shader_log(GLuint shader, const char *name);

void sprite_render(struct sprite *sprite, struct graphics *g);

#endif
