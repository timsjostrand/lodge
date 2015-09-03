/**
 * Basic drawing functionality.
 *
 * Authors: Tim Sjöstrand <tim.sjostrand@gmail.com>
 *          Johan Yngman <johan.yngman@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "graphics.h"
#include "math4.h"
#include "texture.h"

#define VERTICES_RECT_LEN 30

const float vertices_rect[] = {  
    // Vertex            // Texcoord
    -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // Top-left?
    -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Bottom-left ?
     0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top-right ?
     0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top-right ?
    -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Bottom-left ?
     0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // Bottom-right ?
};

#ifdef EMSCRIPTEN
struct graphics* graphics_global;
#endif

void sprite_render(struct sprite *sprite, struct graphics *g)
{
    glEnableVertexAttribArray(0);

    glBindVertexArray(g->vao_rect);

    // Position, rotation and scale
    mat4 transform_position;
    translate(transform_position, xyz(sprite->pos));
    
    mat4 transform_scale;
    scale(transform_scale, xyz(sprite->scale));
    
    mat4 transform_rotation;
    rotate_z(transform_rotation, sprite->rotation);

    mat4 transform_final;
    mult(transform_final, transform_position, transform_rotation);
    mult(transform_final, transform_final, transform_scale);
    transpose_same(transform_final);
    
    // Upload matrices and color
    glUniformMatrix4fv(g->shader.uniform_transform, 1, GL_FALSE, transform_final);
    glUniformMatrix4fv(g->shader.uniform_projection, 1, GL_FALSE, g->projection);
    glUniform4fv(g->shader.uniform_color, 1, sprite->color);
    glUniform1i(g->shader.uniform_sprite_type, sprite->type);
    glUniform1i(g->shader.uniform_tex, 0);

    // Render it!
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int shader_program_log(GLuint program, const char *name)
{
    graphics_debug("=== %s ===\n", name);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(status == GL_FALSE) {
        graphics_error("Link failed\n");
    }

    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    if(len > 0) {
        GLchar *msg = (GLchar *) malloc(len);
        glGetProgramInfoLog(program, len, &len, msg);
        /* TODO: readline() and output for each line */
        if(status == GL_FALSE) {
            graphics_error("%s", msg);
        } else {
            graphics_debug("%s", msg);
        }
        free(msg);
    }

    GLint uniforms = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
    graphics_debug("%d active uniforms\n", uniforms);

    if(status == GL_FALSE) {
        return GRAPHICS_SHADER_ERROR;
    }

    return GRAPHICS_OK;
}

int shader_log(GLuint shader, const char *name)
{
    graphics_debug("=== %s ===\n", name);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status == GL_FALSE) {
        graphics_error("Compilation of %s failed\n", name);
    }

    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    if(len > 0) {
        GLchar *msg = (GLchar *) malloc(len);
        glGetShaderInfoLog(shader, len, &len, msg);
        /* TODO: readline() and output for each line */
        if(status == GL_FALSE) {
            graphics_error("%s", msg);
        } else {
            graphics_debug("%s", msg);
        }
        free(msg);
    }

    if(status == GL_FALSE) {
        glDeleteShader(shader);
        return GRAPHICS_SHADER_ERROR;
    }

    return GRAPHICS_OK;
}

int shader_init(struct shader *s, const char **vertex_shader_src,
        const char **fragment_shader_src, const char **uniform_names,
        int uniforms_count)
{
    int ret = 0;

    /* Compile vertex shader. */
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, vertex_shader_src, NULL);
    glCompileShader(vs);
    ret = shader_log(vs, "vertex shader");

    if(ret != GRAPHICS_OK) {
        return ret;
    }

    /* Compile fragment shader. */
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, fragment_shader_src, NULL);
    glCompileShader(fs);
    ret = shader_log(fs, "fragment shader");

    if(ret != GRAPHICS_OK) {
        return ret;
    }

    /* Compile shader. */
    s->program = glCreateProgram();
    glAttachShader(s->program, fs);
    glAttachShader(s->program, vs);
    glLinkProgram(s->program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    ret = shader_program_log(s->program, "program");

    if(ret != GRAPHICS_OK) {
        return ret;
    }

    /* Set up global uniforms. */
    s->uniform_transform = glGetUniformLocation(s->program, UNIFORM_TRANSFORM_NAME);
    graphics_debug("uniform: %s=%d\n", UNIFORM_TRANSFORM_NAME, s->uniform_transform);
    s->uniform_projection = glGetUniformLocation(s->program, UNIFORM_PROJECTION_NAME);
    graphics_debug("uniform: %s=%d\n", UNIFORM_PROJECTION_NAME, s->uniform_projection);
    s->uniform_color = glGetUniformLocation(s->program, UNIFORM_COLOR_NAME);
    graphics_debug("uniform: %s=%d\n", UNIFORM_COLOR_NAME, s->uniform_color);
    s->uniform_sprite_type = glGetUniformLocation(s->program, UNIFORM_SPRITE_TYPE_NAME);
    graphics_debug("uniform: %s=%d\n", UNIFORM_SPRITE_TYPE_NAME, s->uniform_sprite_type);
    s->uniform_tex = glGetUniformLocation(s->program, UNIFORM_TEX_NAME);
    graphics_debug("uniform: %s=%d\n", UNIFORM_TEX_NAME, s->uniform_tex);

    /* Set up user uniforms. */
    /* NOTE: non-existing uniforms do not result in an error being returned. */
    s->uniforms = (GLint *) malloc(uniforms_count * sizeof(GLint));
    for(int i=0; i<uniforms_count; i++) {
        const char *name = uniform_names[i];
        s->uniforms[i] = glGetUniformLocation(s->program, name);
        graphics_debug("uniform: %s=%d\n", name, s->uniforms[i]);
    }

    return GRAPHICS_OK;
}

void shader_free(struct shader *s)
{
    free(s->uniforms);
    glDeleteProgram(s->program);
}

static int graphics_opengl_init(struct graphics *g, int view_width, int view_height)
{
    /* Global transforms. */
    translate(g->translate, 0.0f, 0.0f, 0.0f);
    scale(g->scale, 10.0f, 10.0f, 1);
    rotate_z(g->rotate, 0);
    ortho(g->projection, 0, view_width, view_height, 0, -1.0f, 1.0f);
    transpose_same(g->projection);

    /* OpenGL. */
    // glViewport( 0, 0, view_width, view_height );
    glClearColor(0.33f, 0.33f, 0.33f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Vertex buffer. */
    glGenBuffers(1, &g->vbo_rect);
    glBindBuffer(GL_ARRAY_BUFFER, g->vbo_rect);
    glBufferData(GL_ARRAY_BUFFER, VERTICES_RECT_LEN * sizeof(float),
            vertices_rect, GL_STATIC_DRAW);

    /* Vertex array. */
    glGenVertexArrays(1, &g->vao_rect);
    glBindVertexArray(g->vao_rect);

    /* Position stream. */
    GLint posAttrib = glGetAttribLocation(g->shader.program, "vp");
    graphics_debug("attrib: vp=%d\n", posAttrib);
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    /* Texcoord stream. */
    GLint texcoordAttrib = glGetAttribLocation(g->shader.program, "texcoord_in");
    graphics_debug("attrib: texcoord_in=%d\n", texcoordAttrib);
    glEnableVertexAttribArray(texcoordAttrib);
    glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*) (3 * sizeof(float)));

    return GRAPHICS_OK;
}

void graphics_glfw_resize_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int graphics_libraries_init(struct graphics *g, int view_width, int view_height,
        int windowed)
{
    /* Initialize the library */
    if(!glfwInit()) {
        return GRAPHICS_GLFW_ERROR;
    }

#ifdef EMSCRIPTEN
    g->window = glfwCreateWindow(view_width, view_height, "glpong", NULL, NULL);
    if(!g->window) {
        glfwTerminate();
        return GRAPHICS_GLFW_ERROR;
    }
#else
    /* QUIRK: Mac OSX */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *video_mode = glfwGetVideoMode(monitor);

    /* Create a windowed mode window and its OpenGL context */
    if(windowed) {
        g->window = glfwCreateWindow(view_width, view_height, "glpong", NULL, NULL);
    } else {
        g->window = glfwCreateWindow(video_mode->width, video_mode->height, "glpong", monitor, NULL);
    }
    if(!g->window) {
        glfwTerminate();
        return GRAPHICS_GLFW_ERROR;
    }
#endif

    /* Be notified when window size changes. */
    glfwSetFramebufferSizeCallback(g->window, &graphics_glfw_resize_callback);

    /* Select the current OpenGL context. */
    glfwMakeContextCurrent(g->window);

    /* Init GLEW. */
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        return GRAPHICS_GLEW_ERROR;
    }

	return GRAPHICS_OK;
}

/**
 *
 * @param g                     A graphics struct to fill in.
 * @param view_width            The width of the view, used for ortho().
 * @param view_height           The height of the view, used for ortho().
 * @param windowed              If applicable, whether to start in windowed mode.
 * @param vertex_shader_src     Vertex shader source.
 * @param fragment_shader_src   Fragment shader source.
 * @param uniform_names         A list of uniform names in the shader,
 *                              excluding 'transform', 'projection' and 'color'.
 * @param uniforms_count        Number of elements in the uniform name list.
 */
int graphics_init(struct graphics *g, think_func_t think, render_func_t render,
        int view_width, int view_height, int windowed,
        const char **vertex_shader_src, const char **fragment_shader_src,
        const char **uniform_names, int uniforms_count)
{
    int ret = 0;

#ifdef EMSCRIPTEN
    /* HACK: Need to store reference for emscripten. */
    graphics_global = g;
#endif

    /* Set up the graphics struct properly. */
    g->delta_time_factor = 1.0f;
    g->think = think;
    g->render = render;

    /* Set up GLEW and glfw. */
    ret = graphics_libraries_init(g, view_width, view_height, windowed);

    if(ret != GRAPHICS_OK) {
        return ret;
    }

    /* Set up shader. */
    ret = shader_init(&g->shader, vertex_shader_src, fragment_shader_src,
            uniform_names, uniforms_count);

    if(ret != GRAPHICS_OK) {
        return ret;
    }

    /* Set up OpenGL. */
    ret = graphics_opengl_init(g, view_width, view_height);

    if(ret != GRAPHICS_OK) {
        return ret;
    }

    return GRAPHICS_OK;
}

void graphics_free(struct graphics *g)
{
    /* Free resources. */
    glDeleteVertexArrays(1, &g->vao_rect);
    glDeleteBuffers(1, &g->vbo_rect);

    /* Free shader. */
    shader_free(&g->shader);

    /* Shut down glfw. */
    glfwTerminate();
}

void graphics_count_frame(struct graphics *g)
{
    g->frames ++;
    if(glfwGetTime()*1000.0  - g->last_frame_report >= 1000.0) {
        graphics_debug("%d FPS\n", g->frames);
        g->last_frame_report = glfwGetTime()*1000.0f;
        g->frames = 0;
    }
}

static void shader_think(struct graphics *g, float delta_time)
{
    /* Upload transform uniform. */
    mat4 transform;
    mult(transform, g->translate, g->scale);
    mult(transform, transform, g->rotate);
    transpose_same(transform);
    glUniformMatrix4fv(g->shader.uniform_transform, 1, GL_FALSE, transform);

    /* Upload projection uniform. */
    glUniformMatrix4fv(g->shader.uniform_projection, 1, GL_FALSE, g->projection);
}

void graphics_do_frame(struct graphics *g)
{     
    /* Delta-time. */
    float delta_time = 0;
    if(g->last_frame != 0) {
        delta_time = (glfwGetTime() - g->last_frame) * 1000.0f * g->delta_time_factor;
    }
    g->last_frame = glfwGetTime();

    /* Game loop. */
    g->think(g, delta_time);
    shader_think(g, delta_time);
    g->render(g, delta_time);

    /* Swap front and back buffers */
    glfwSwapBuffers(g->window);

    /* Poll for and process events */
    glfwPollEvents();

    /* Register that a frame has been drawn. */
    graphics_count_frame(g);
}

#ifdef EMSCRIPTEN
void graphics_do_frame_emscripten()
{
	graphics_do_frame(graphics_global);
}
#endif

void graphics_loop(struct graphics *g)
{
    /* Sanity check. */
    if(!g->render || !g->think) {
        graphics_error("g->render() or g->think() not set!\n");
        return;
    }

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(graphics_do_frame_emscripten, 0, 1);
#else
    while(!glfwWindowShouldClose(g->window)) {
        graphics_do_frame(g);
    }
#endif
}
