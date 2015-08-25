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

const float vertices_rect[] = {  
    -0.5f,  0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f,
     0.5f,  0.5f,  0.0f,
     0.5f,  0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f,
     0.5f, -0.5f,  0.0f
};

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

    // Render it!
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void shader_program_log(GLuint program, const char *name)
{
    printf("=== %s ===\n", name);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if(success == GL_FALSE) {
        printf("FAILED\n");
    }

    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    if(len > 0) {
        GLchar *msg = (GLchar *) malloc(len);
        glGetProgramInfoLog(program, len, &len, msg);
        printf("%s", msg);
        free(msg);
    }

    GLint uniforms = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
    printf("%d active uniforms\n", uniforms);
}

void shader_log(GLuint shader, const char *name)
{
    printf("=== %s ===\n", name);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE) {
        printf("FAILED\n");
    }

    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    if(len > 0) {
        GLchar *msg = (GLchar *) malloc(len);
        glGetShaderInfoLog(shader, len, &len, msg);
        printf("%s", msg);
        free(msg);
    }

    if(success == GL_FALSE) {
        glDeleteShader(shader);
    }
}

int shader_init(struct shader *s, const char **vertex_shader_src,
        const char **fragment_shader_src, const char **uniform_names,
        int uniforms_count)
{
    /* Vertex shader. */
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, vertex_shader_src, NULL);
    glCompileShader(vs);
    shader_log(vs, "vertex shader");

    /* Fragment shader. */
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, fragment_shader_src, NULL);
    glCompileShader(fs);
    shader_log(fs, "fragment shader");

    /* Compile shader. */
    s->program = glCreateProgram();
    glAttachShader(s->program, fs);
    glAttachShader(s->program, vs);
    glLinkProgram(s->program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    shader_program_log(s->program, "program");

    /* Set up global uniforms. */
    s->uniform_transform = glGetUniformLocation(s->program, UNIFORM_TRANSFORM_NAME);
    printf("uniform: %s=%d\n", UNIFORM_TRANSFORM_NAME, s->uniform_transform);
    s->uniform_projection = glGetUniformLocation(s->program, UNIFORM_PROJECTION_NAME);
    printf("uniform: %s=%d\n", UNIFORM_PROJECTION_NAME, s->uniform_projection);
    s->uniform_color = glGetUniformLocation(s->program, UNIFORM_COLOR_NAME);
    printf("uniform: %s=%d\n", UNIFORM_COLOR_NAME, s->uniform_color);
    s->uniform_sprite_type = glGetUniformLocation(s->program, UNIFORM_SPRITE_TYPE_NAME);
    printf("uniform: %s=%d\n", UNIFORM_SPRITE_TYPE_NAME, s->uniform_sprite_type);

    /* Set up user uniforms. */
    s->uniforms = (GLint *) malloc(uniforms_count * sizeof(GLint));
    for(int i=0; i<uniforms_count; i++) {
        const char *name = uniform_names[i];
        s->uniforms[i] = glGetUniformLocation(s->program, name);
        printf("uniform: %s=%d\n", name, s->uniforms[i]);
    }

    return 0;
}

void shader_free(struct shader *s)
{
    free(s->uniforms);
    glDeleteProgram(s->program);
}

void graphics_opengl_init(struct graphics *g, int view_width, int view_height,
        const char **vertex_shader_src, const char **fragment_shader_src,
        const char **uniform_names, int uniforms_count)
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
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), vertices_rect, GL_STATIC_DRAW);

    /* Vertex array. */
    glGenVertexArrays(1, &g->vao_rect);
    glBindVertexArray(g->vao_rect);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g->vbo_rect);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void graphics_glfw_resize_callback(GLFWwindow *window, int width, int height)
{
   printf("TODO: resize callback\n");
}

int graphics_libraries_init(struct graphics *g, int view_width, int view_height,
        int windowed)
{
    /* Initialize the library */
    if(!glfwInit()) {
        return -1;
    }

#ifdef EMSCRIPTEN
    g->window = glfwCreateWindow(view_width, view_height, "glpong", NULL, NULL);
    if(!g->window) {
        glfwTerminate();
        return -1;
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
        return -1;
    }
#endif

    /* Be notified when window size changes. */
    glfwSetWindowSizeCallback(g->window, &graphics_glfw_resize_callback);

    /* Select the current OpenGL context. */
    glfwMakeContextCurrent(g->window);

    /* Init GLEW. */
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        printf("glewInit() failed\n");
        return -1;
    }

	return 0;
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
    /* Set up the graphics struct properly. */
    g->delta_time_factor = 1.0f;
    g->think = think;
    g->render = render;

    /* Set up GLEW and glfw. */
    graphics_libraries_init(g, view_width, view_height, windowed);

    /* Set up OpenGL. */
    graphics_opengl_init(g, view_width, view_height, vertex_shader_src,
            fragment_shader_src, uniform_names, uniforms_count);

    /* Set up shader. */
    shader_init(&g->shader, vertex_shader_src, fragment_shader_src,
            uniform_names, uniforms_count);

    return 0;
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
        printf("%d FPS\n", g->frames);
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
struct graphics* em_graphics;
void graphics_do_frame_emscripten()
{
	graphics_do_frame(em_graphics);
}
#endif

void graphics_loop(struct graphics *g)
{
    /* Sanity check. */
    if(!g->render || !g->think) {
        printf("ERROR: g->render() or g->think() not set!\n");
        return;
    }

#ifdef EMSCRIPTEN
	em_graphics = g;
	emscripten_set_main_loop(graphics_do_frame_emscripten, 0, 1);
#else
    while(!glfwWindowShouldClose(g->window)) {
        graphics_do_frame(g);
    }
#endif
}
