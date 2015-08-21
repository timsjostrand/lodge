/**
 * OpenGL Pong!
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
#ifndef _WIN32
#define GLFW_INCLUDE_GLCOREARB
#endif 
#include <GLFW/glfw3.h>

#include "math4.h"

const float vertices_rect[] = 
{  
    -0.5f,  0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f,
     0.5f,  0.5f,  0.0f,
     0.5f,  0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f,
     0.5f, -0.5f,  0.0f
};

#define VIEW_WIDTH      640
#define VIEW_HEIGHT     360     /* 16:9 aspect ratio */

#define BOARD_TOP       VIEW_HEIGHT
#define BOARD_BOTTOM    0
#define BOARD_LEFT      0
#define BOARD_RIGHT     VIEW_WIDTH

#define BALL_WIDTH      16
#define BALL_HEIGHT     16

#define PLAYER_WIDTH    16
#define PLAYER_HEIGHT   64
#define PLAYER1_HIT     32.0f
#define PLAYER2_HIT     608.0f

#define PLAYER_CHARGE   0.01f

struct sprite {
    vec4 pos;
    vec4 scale;
    vec4 color;
    float rotation;
};

struct stats {
    int points;
    int hits;
    int streak;
    int current_streak;
};

struct player {
    struct sprite   sprite;
    struct stats    stats;
    float           charge;
};

struct ball {
    struct sprite   sprite;
    float           vx;
    float           vy;
    float           speed;
    float           last_hit;
};

struct shader {
    GLuint  program;
    GLint   uniform_time;
    GLint   uniform_transform;
    GLint   uniform_projection;
    GLint   uniform_color;
    GLint   uniform_is_ball;
    GLint   uniform_ball_last_hit;
};

struct game {
    double          time;                       /* Time since start. */
    float           time_mod;                   /* Time delta factor. */
    GLuint          vbo;                        /* Vertex Buffer Object. */
    GLuint          vao;                        /* Vertex Array Object. */
    struct shader   shader;                     /* Shader program information. */
    mat4            projection;                 /* Projection matrix. */
    mat4            translate;                  /* Global translation matrix. */
    mat4            rotate;                     /* Global rotation matrix. */
    mat4            scale;                      /* Global scale matrix. */
    struct player   player1;                    /* Left player. */
    struct player   player2;                    /* Right player. */
    struct ball     ball;
    struct stats    total_stats;
    int             keys[GLFW_KEY_LAST];        /* Key status of current frame. */
    int             last_keys[GLFW_KEY_LAST];   /* Key status of last frame. */
} game = { 0 };

const char *vertex_shader =
    "#version 400\n"
    "uniform mat4 transform;"
    "uniform mat4 projection;"
    "uniform float time;"
    "uniform int is_ball;"
    "uniform float ball_last_hit;"
    "const vec4 oposes[6] = vec4[6] ("
    "   vec4( 0.25,  0.5, 0.0, 0.0),"
    "   vec4( 0.25, -0.5, 0.0, 0.0),"
    "   vec4(-0.25,  0.5, 0.0, 0.0),"
    "   vec4(-0.25,  0.5, 0.0, 0.0),"
    "   vec4( 0.25, -0.5, 0.0, 0.0),"
    "   vec4(-0.25, -0.5, 0.0, 0.0)"
    ");"
    "in vec3 vp;"
    "void main() {"
    "   vec4 opos = oposes[gl_VertexID];"
    "   float bh = ball_last_hit;"
    "   if(bh <= 0.016)"
    "       bh = 0.016;"
    "   float decay = 0.016 / (bh*0.0001);"
    "   decay = clamp(decay, 0.0, 1.5);"
    "   float hit_wobble = abs(cos(bh/80.0)) * decay;"
    "   gl_Position = projection * transform * ("
    "       vec4(vp, 1.0)"
    "       + opos*is_ball*hit_wobble"
    "       + 0.0*opos*is_ball*cos(time*8.0f)/6.0"
    "   );"
    "}";

const char *fragment_shader =
    "#version 400\n"
    "uniform float time;"
    "uniform vec4 color;"
    "out vec4 frag_color;"
    "void main() {"
    "   frag_color = color;"
    "}";

void sprite_render(struct sprite *sprite, struct game *game)
{
    glEnableVertexAttribArray(0);

    glBindVertexArray(game->vao);

    // Position, rotation and scale
    mat4 transform_position;
    translate(transform_position, sprite->pos[0], sprite->pos[1], sprite->pos[2]);
    
    mat4 transform_scale;
    scale(transform_scale, sprite->scale[0], sprite->scale[1], sprite->scale[2]);
    
    mat4 transform_rotation;
    rotate(transform_rotation, sprite->rotation);

    mat4 transform_final;
    mult(transform_final, transform_position, transform_rotation);
    mult(transform_final, transform_final, transform_scale);
    
    // Upload matrices and color
    glUniformMatrix4fv(game->shader.uniform_transform, 1, GL_TRUE, transform_final);
    glUniformMatrix4fv(game->shader.uniform_projection, 1, GL_TRUE, game->projection);
    glUniform4fv(game->shader.uniform_color, 1, sprite->color);

    // Render it!
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int key_down(int key)
{
    return game.keys[key];
}

int key_pressed(int key)
{
    return game.keys[key] && !game.last_keys[key];
}

int key_released(int key)
{
    return !game.keys[key] && game.last_keys[key];
}

void print_stats()
{
    printf("Player 1: Points: %-2d, Hits: %-2d, Longest Streak: %-2d\n",
            game.player1.stats.points,
            game.player1.stats.hits,
            game.player1.stats.streak);
    printf("Player 2: Points: %-2d, Hits: %-2d, Longest Streak: %-2d\n",
            game.player2.stats.points,
            game.player2.stats.hits,
            game.player2.stats.streak);
    printf("Longest total streak: %-2d\n", game.total_stats.streak);
}

void game_think(float dt)
{
    // Ball: left/right hit detection
    int ball_reset = 0;
    if(game.ball.sprite.pos[0] < BOARD_LEFT + 8) {
        ball_reset = 1;
        game.total_stats.current_streak = 0;
        game.player1.stats.current_streak = 0;
        game.player2.stats.points ++;
        printf("player 1 looses \n");
        print_stats();
    } else if(game.ball.sprite.pos[0] > BOARD_RIGHT - 8) {
        ball_reset = 1;
        game.total_stats.current_streak = 0;
        game.player2.stats.current_streak = 0;
        game.player1.stats.points ++;
        printf("player 2 looses \n");
        print_stats();
    }

    if(ball_reset) {
        /* Shameball! */
        game.ball.vx = clamp(game.ball.vx / 4.0f,
                -game.ball.speed / 8.0f,
                game.ball.speed / 8.0f);
        game.ball.vy = clamp(game.ball.vy / 4.0f,
                -game.ball.speed / 8.0f,
                game.ball.speed / 8.0f);
        printf("game.ball.vy=%6f\n", game.ball.vy);
        /* Restart at center of view. */
        game.ball.sprite.pos[0] = VIEW_WIDTH/2;
        game.ball.sprite.pos[1] = VIEW_HEIGHT/2;
    }
        
    // Ball: top/bottom hit detection
    if(game.ball.sprite.pos[1] > BOARD_TOP - BALL_HEIGHT/2) {
        game.ball.sprite.pos[1] = BOARD_TOP - BALL_HEIGHT/2;
        game.ball.vy *= -1.0f;
        game.ball.last_hit = 0.0f;
    } else if(game.ball.sprite.pos[1] < BOARD_BOTTOM + BALL_HEIGHT/2) {
        game.ball.sprite.pos[1] = BOARD_BOTTOM + BALL_HEIGHT/2;
        game.ball.vy *= -1.0f;
        game.ball.last_hit = 0.0f;
    }

    // Ball: move
    game.ball.sprite.pos[0] += dt*game.ball.vx;
    game.ball.sprite.pos[1] += dt*game.ball.vy;
    game.ball.last_hit += dt;

    game.player1.charge += dt * PLAYER_CHARGE;
    game.player2.charge += dt * PLAYER_CHARGE;

    // Paddles
    if(game.keys[GLFW_KEY_W]) {
        game.player1.sprite.pos[1] += dt*0.6f;
        game.player1.charge = game.player1.charge - 16.0f * dt * PLAYER_CHARGE;
    } else if(game.keys[GLFW_KEY_S]) {
        game.player1.sprite.pos[1] -= dt*0.6f;
        game.player1.charge = game.player1.charge - 16.0f * dt * PLAYER_CHARGE;
    }

    if(game.keys[GLFW_KEY_UP]) {
        game.player2.sprite.pos[1] += dt*0.6f;
        game.player2.charge = game.player2.charge - 16.0f * dt * PLAYER_CHARGE;
    } else if(game.keys[GLFW_KEY_DOWN]) {
        game.player2.sprite.pos[1] -= dt*0.6f;
        game.player2.charge = game.player2.charge - 16.0f * dt * PLAYER_CHARGE;
    }

    // Cap charge
    game.player1.charge = clamp(game.player1.charge, 0.0f, 32.0f);
    game.player2.charge = clamp(game.player2.charge, 0.0f, 32.0f);

    // If paddle is outside board
    game.player1.sprite.pos[1] = clamp(game.player1.sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
    game.player2.sprite.pos[1] = clamp(game.player2.sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);

    // Game logic

    // Ball collides with Player 1?
    if(game.ball.vx < 0
            && game.ball.sprite.pos[1] >= (game.player1.sprite.pos[1] - PLAYER_HEIGHT/2)
            && game.ball.sprite.pos[1] <= (game.player1.sprite.pos[1] + PLAYER_HEIGHT/2)
            && game.ball.sprite.pos[0] <= (PLAYER1_HIT + PLAYER_WIDTH/2 + BALL_WIDTH/2)) {
        printf("collides with player 1\n");
        game.player1.stats.hits ++;
        game.player1.stats.current_streak ++;
        game.player1.stats.streak = imax(game.player1.stats.streak, game.player1.stats.current_streak);
        game.total_stats.current_streak ++;
        game.total_stats.streak = imax(game.total_stats.streak, game.total_stats.current_streak);

        float diff = (game.ball.sprite.pos[1] - game.player1.sprite.pos[1]) / (PLAYER_HEIGHT/2.0f);
        diff = clamp(diff, -0.8f, 0.8f);
        float angle = M_PI/2.0f - acos(diff);
        float force = 0.5f + game.player1.charge/16.0f;
        float current_speed = sqrt( game.ball.vx*game.ball.vx + game.ball.vy*game.ball.vy);
        current_speed = fmax(0.25f, current_speed);
        game.ball.vx = cos(angle) * current_speed * force;
        game.ball.vy = sin(angle) * current_speed * force;
        game.ball.last_hit = 0.0f;

        game.player1.charge = 0.0f;
    }

    // Ball collides with Player 2?
    if(game.ball.vx > 0
            && game.ball.sprite.pos[1] >= (game.player2.sprite.pos[1] - PLAYER_HEIGHT/2)
            && game.ball.sprite.pos[1] <= (game.player2.sprite.pos[1] + PLAYER_HEIGHT/2)
            && game.ball.sprite.pos[0] >= (PLAYER2_HIT - PLAYER_WIDTH/2 - BALL_WIDTH/2)) {
        printf("collides with player 2\n");
        game.player2.stats.hits ++;
        game.player2.stats.current_streak ++;
        game.player2.stats.streak = imax(game.player2.stats.streak, game.player2.stats.current_streak);
        game.total_stats.current_streak ++;
        game.total_stats.streak = imax(game.total_stats.streak, game.total_stats.current_streak);

        float diff = (game.ball.sprite.pos[1] - game.player2.sprite.pos[1]) / (PLAYER_HEIGHT/2.0f);
        diff = clamp(diff, -0.8f, 0.8f);
        float angle =  M_PI/2.0f + acos(diff);
        float force = 0.5f + game.player1.charge/16.0f;
        float current_speed = sqrt( game.ball.vx*game.ball.vx + game.ball.vy*game.ball.vy);
        current_speed = fmax(0.25f, current_speed); // So much code duplication!!
        game.ball.vx = cos(angle) * current_speed * force;
        game.ball.vy = sin(angle) * current_speed * force;
        game.ball.last_hit = 0.0f;
    
        game.player2.charge = 0.0f;
    }

    // Set player color
    game.player1.sprite.color[1] = 1.0f - game.player1.charge/32.0f;
    game.player1.sprite.color[2] = 1.0f - game.player1.charge/32.0f;
    game.player2.sprite.color[1] = 1.0f - game.player2.charge/32.0f;
    game.player2.sprite.color[2] = 1.0f - game.player2.charge/32.0f;
}

void shader_think(float delta_time)
{
    /* Upload uniforms. */
    glUniform1f(game.shader.uniform_time, (GLfloat) game.time);

    /* Transform. */
    mat4 transform;
	mult(transform, game.translate, game.scale);
    mult(transform, transform, game.rotate);
    glUniformMatrix4fv(game.shader.uniform_transform, 1, GL_TRUE, transform);

    /* Projection. */
    glUniformMatrix4fv(game.shader.uniform_projection, 1, GL_TRUE, game.projection);

    /* Ball stuff */
    glUniform1f(game.shader.uniform_ball_last_hit, game.ball.last_hit);
}

void think(float delta_time)
{
    shader_think(delta_time);
    game_think(delta_time);

    /* Remember what keys were pressed the last frame. */
    memcpy(game.last_keys, game.keys, GLFW_KEY_LAST);
}

void render(GLFWwindow *window)
{
    //glEnableClientState(0);
    glEnableVertexAttribArray(0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(game.shader.program);

    glUniform1i(game.shader.uniform_is_ball, 0);
    sprite_render(&game.player1.sprite, &game);
    sprite_render(&game.player2.sprite, &game);
    
    glUniform1i(game.shader.uniform_is_ball, 1);
    sprite_render(&game.ball.sprite, &game);
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
    printf("%d uniforms\n", uniforms);
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

void resize(GLFWwindow *window, int width, int height)
{
}

void init_shaders(struct shader *s)
{
    /* Vertex shader. */
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    shader_log(vs, "vertex shader");

    /* Fragment shader. */
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
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

    /* Set up uniforms. */
    s->uniform_time = glGetUniformLocation(s->program, "time");
    s->uniform_transform = glGetUniformLocation(s->program, "transform");
    s->uniform_projection = glGetUniformLocation(s->program, "projection");
    s->uniform_color = glGetUniformLocation(s->program, "color");
    s->uniform_is_ball = glGetUniformLocation(s->program, "is_ball");
    s->uniform_ball_last_hit = glGetUniformLocation(s->program, "ball_last_hit");
    printf("uniform_time=%d\n", s->uniform_time);
    printf("uniform_transform=%d\n", s->uniform_transform);
    printf("uniform_projection=%d\n", s->uniform_projection);
    printf("uniform_color=%d\n", s->uniform_color);
    printf("uniform_is_ball=%d\n", s->uniform_is_ball);
    printf("uniform_ball_last_hit=%d\n", s->uniform_ball_last_hit);
}

void init_player1(struct player *p)
{
    p->sprite.pos[0] = 32.0f;
    p->sprite.pos[1] = 50.0f;
    p->sprite.pos[2] = 0.0f;
    p->sprite.pos[3] = 1.0f;
   
    p->sprite.color[0] = 1.0f;
    p->sprite.color[1] = 1.0f;
    p->sprite.color[2] = 1.0f;
    p->sprite.color[3] = 1.0f;

    p->sprite.scale[0] = PLAYER_WIDTH;
    p->sprite.scale[1] = PLAYER_HEIGHT;
    p->sprite.scale[2] = 1.0f;
    p->sprite.scale[3] = 1.0f;
}

void init_player2(struct player *p)
{
    p->sprite.pos[0] = 608.0f;
    p->sprite.pos[1] = 50.0f;
    p->sprite.pos[2] = 0.0f;
    p->sprite.pos[3] = 1.0f;
    
    p->sprite.color[0] = 1.0f;
    p->sprite.color[1] = 1.0f;
    p->sprite.color[2] = 1.0f;
    p->sprite.color[3] = 1.0f;
  
    p->sprite.scale[0] = PLAYER_WIDTH;
    p->sprite.scale[1] = PLAYER_HEIGHT;
    p->sprite.scale[2] = 1.0f;
    p->sprite.scale[3] = 1.0f;
}

void init_ball(struct ball *ball)
{
    ball->speed = 0.6f;
    float random_angle = ((float)(((int)rand())%(int)((int)2*M_PI)*1000))/1000.0f;
    ball->vy = ball->speed * sin(random_angle) / 4.0f;
    ball->vx = ball->speed * cos(random_angle) / 4.0f;

    ball->sprite.pos[0] = VIEW_WIDTH/2;
    ball->sprite.pos[1] = VIEW_HEIGHT/2;
    ball->sprite.pos[2] = 0.0f;
    ball->sprite.pos[3] = 1.0f;
    
    ball->sprite.scale[0] = BALL_WIDTH;
    ball->sprite.scale[1] = BALL_HEIGHT;
    ball->sprite.scale[2] = 1.0f;
    ball->sprite.scale[3] = 1.0f;
    
    ball->sprite.color[0] = 1.0f;
    ball->sprite.color[1] = 1.0f;
    ball->sprite.color[2] = 1.0f;
    ball->sprite.color[3] = 1.0f;
  
    ball->last_hit = 10.0f;
}

void init()
{
    /* Game setup. */
    game.time_mod = 1.0f;
    translate(game.translate, 0.0f, 0.0f, 0.0f);
    scale(game.scale, 10.0f, 10.0f, 1);
    rotate(game.rotate, 0);
    ortho(game.projection, 0, VIEW_WIDTH, VIEW_HEIGHT, 0, -1.0f, 1.0f);

    /* Entities. */
    init_player1(&game.player1);
    init_player2(&game.player2);
    init_ball(&game.ball);

    /* Set up OpenGL. */
    glClearColor(0.33f, 0.33f, 0.33f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
//    glDisable(GL_CULL_FACE);

    /* Create vertex buffer. */
    glGenBuffers(1, &game.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, game.vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), vertices_rect, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &game.vao);
    glBindVertexArray(game.vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, game.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    init_shaders(&game.shader);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    /* Sanity check */
    if(key < 0 || key > GLFW_KEY_LAST) {
        printf("Invalid key: %d (scancode=%d)\n", key, scancode);
        return;
    }

    /* Only care about 'up'/'down', regard 'repeat' as 'down'. */
    game.keys[key] = !(action == 0);

    if(action == GLFW_RELEASE) {
        switch(key) {
            case GLFW_KEY_O:
                game.time_mod *= 2.0f;
                printf("game.time_mod=%f\n", game.time_mod);
                break;
            case GLFW_KEY_P:
                game.time_mod /= 2.0f;
                printf("game.time_mod=%f\n", game.time_mod);
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, 1);
                break;
        }
    }
}

void clean_up()
{
    glDeleteVertexArrays(1, &game.vao);
    glDeleteBuffers(1, &game.vbo);
    glDeleteProgram(game.shader.program);
    glfwTerminate();
}

int main(int argc, char **argv)
{
    srand(time(0));

    GLFWwindow *window;

    /* Initialize the library */
    if(!glfwInit()) {
        return -1;
    }

    /* QUIRK: Mac OSX */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *pMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *pVideomode = glfwGetVideoMode(pMonitor); // TODO: Use to set resolution?

    /* Create a windowed mode window and its OpenGL context */
    //window = glfwCreateWindow(VIEW_WIDTH, VIEW_HEIGHT, "glpong", NULL, NULL);
    window = glfwCreateWindow(pVideomode->width, pVideomode->height, "glpong", pMonitor, NULL);
    if(!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Init GLEW. */
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        printf("glewInit() failed\n");
        return -1;
    }

    /* Init OpenGL. */
    init();

    /* Get input events. */
    glfwSetKeyCallback(window, &key_callback);
    glfwSetWindowSizeCallback(window, &resize);

    /* Loop until the user closes the window */
    while(!glfwWindowShouldClose(window)) { 
        /* Delta-time. */
        float delta_time = 0;
        if(game.time != 0) {
            delta_time = (glfwGetTime() - game.time) * 1000.0f * game.time_mod;
        }
        game.time = glfwGetTime();

        /* Game loop. */
        think(delta_time);
        render(window);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    clean_up();
    return 0;
}
