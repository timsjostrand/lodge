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
#include <GLFW/glfw3.h>
#ifdef EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "math4.h"
#include "color.h"
#include "graphics.h"
#include "input.h"

#define VIEW_WIDTH      640
#define VIEW_HEIGHT     360     /* 16:9 aspect ratio */

#define BOARD_TOP       VIEW_HEIGHT
#define BOARD_BOTTOM    0
#define BOARD_LEFT      0
#define BOARD_RIGHT     VIEW_WIDTH

#define BALL_WIDTH      16
#define BALL_HEIGHT     16
#define BALL_SPEED_MAX  0.5f

#define PLAYER_WIDTH    16
#define PLAYER_HEIGHT   64
#define PLAYER1_HIT     32.0f
#define PLAYER2_HIT     608.0f

#define PARTICLES_MAX   100
#define PARTICLE_ALPHA  0.5f

enum uniforms {
    TIME,
    IS_BALL,
    BALL_LAST_HIT,
    UNIFORM_LAST
};

const char *uniform_names[] = {
    "time",
    "is_ball",
    "ball_last_hit",
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
    float           last_emit;
};

struct ball {
    struct sprite   sprite;
    float           vx;
    float           vy;
    float           speed;
    float           last_hit;
};

struct particle {
    struct sprite   sprite;
    int             dead;
    float           age;
    float           age_max;
    float           vx;
    float           vy;
    float           va;
};

struct game {
    double          time;                       /* Time since start. */
    float           time_mod;                   /* Time delta factor. */
    struct graphics graphics;                   /* Graphics state. */
    struct player   player1;                    /* Left player. */
    struct player   player2;                    /* Right player. */
    struct ball     ball;
    struct stats    total_stats;
    struct particle particles[PARTICLES_MAX];
    int             particles_count;
    struct input    input;
} game = { 0 };

#ifdef EMSCRIPTEN
const char *fragment_shader =
    "#version 100\n"
    "precision mediump float;"
    "uniform float time;"
    "uniform vec4 color;"
    "void main() {"
    "   gl_FragColor = color;"
    "}";

const char *vertex_shader =
    "#version 100\n"
    "uniform mat4 transform;"
    "uniform mat4 projection;"
    "attribute vec3 vp;"
    "void main() {"
    "   gl_Position = projection * transform * vec4(vp, 1.0);"
    "}";
#else
const char *fragment_shader =
    "#version 400\n"
    "uniform float time;"
    "uniform vec4 color;"
    "out vec4 frag_color;"
    "void main() {"
    "   frag_color = color;"
    "}";

const char *vertex_shader =
    "#version 400 core\n"
    "precision highp float;"
    "uniform mat4 transform;"
    "uniform mat4 projection;"
    "uniform float time;"
    "uniform int is_ball;"
    "uniform float ball_last_hit;"
    "const vec4 oposes[6] = vec4[6]("
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
#endif

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

void particle_init(struct particle *p, float x, float y, float w, float h,
        float angle, float vx, float vy, float va, float age_max)
{
    p->dead = 0;
    p->age = 0.0f;
    p->age_max = age_max;
    p->vx = vx;
    p->vy = vy;
    p->va = va;
    p->sprite.rotation = angle;

    setv(p->sprite.pos, x, y, 0.0f, 1.0f);
    setv(p->sprite.color, rgb(COLOR_WHITE), PARTICLE_ALPHA);
    setv(p->sprite.scale, w, h, 1.0f, 1.0f);
}

void particle_new(float x, float y, float w, float h, float angle, float vx,
        float vy, float va, float age_max)
{
    if(game.particles_count >= PARTICLES_MAX) {
        printf("max particle count reached\n");
        return;
    }
    particle_init(&game.particles[game.particles_count], x, y, w, h, angle,
            vx, vy, va, age_max);
    game.particles_count ++;
}

int player_is_charged(struct player *p)
{
    /* p->charge is idle time in ms. */
    return p->charge >= 3200.0f;
}

void think_player_charged(struct player *p, float dt)
{
    if(player_is_charged(p) && p->last_emit >= 16.0f) {
        float size = randr(8.0f, 16.0f);
        particle_new(p->sprite.pos[0],                                          // x
                p->sprite.pos[1] + randr(-PLAYER_HEIGHT/2, PLAYER_HEIGHT/2),    // y
                size, size,                                                     // w, h
                randr(0, 2 * M_PI),                                             // angle
                randr(-0.05f, 0.05f),                                           // vx
                randr(-0.05f, 0.05f),                                           // vy
                randr(-(2 * M_PI) * 0.0001f, (2 * M_PI) * 0.0001f),             // va
                randr(200.0f, 600.0f)                                           // time_max
        );
        p->last_emit = 0;
    }
    p->last_emit += dt;
}

void ball_player_bounce(struct ball *ball, struct player *p)
{
    /* 0 on player1, 1 on player2. */
    int player_no = (p == &game.player2);
    printf("collides with player %d\n", player_no + 1);

    p->stats.hits ++;
    p->stats.current_streak ++;
    p->stats.streak = imax(p->stats.streak, p->stats.current_streak);

    game.total_stats.current_streak ++;
    game.total_stats.streak = imax(game.total_stats.streak, game.total_stats.current_streak);

    float diff = (ball->sprite.pos[1] - p->sprite.pos[1]) / (PLAYER_HEIGHT/2.0f);
    diff = clamp(diff, -0.8f, 0.8f);
    /* angle table:
     * player1: angle = M_PI/2.0f - acos(diff)
     * player2: angle = M_PI/2.0f + acos(diff) */
    float angle = M_PI/2.0f + (player_no == 0 ? -1 : +1) * acos(diff);
    float force = 1.0f + (1.5f * player_is_charged(p));
    float current_speed = sqrt(ball->vx*ball->vx + ball->vy*ball->vy);
    printf("force=%6f, current_speed=%6f\n", force, current_speed);
    current_speed = fmax(BALL_SPEED_MAX, current_speed);
    ball->vx = cos(angle) * current_speed * force;
    ball->vy = sin(angle) * current_speed * force;
    ball->last_hit = 0.0f;

    p->charge = 0.0f;
}

void ball_think(float dt)
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
}

void player1_think(float dt)
{
    game.player1.charge += dt;

    // Move?
    if(key_down(GLFW_KEY_W)) {
        game.player1.sprite.pos[1] += dt*0.6f;
        game.player1.charge = 0;
    } else if(key_down(GLFW_KEY_S)) {
        game.player1.sprite.pos[1] -= dt*0.6f;
        game.player1.charge = 0;
    }

    // Outside board?
    game.player1.sprite.pos[1] = clamp(game.player1.sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void player2_think(float dt)
{
    game.player2.charge += dt;

    // Move?
    if(key_down(GLFW_KEY_UP)) {
        game.player2.sprite.pos[1] += dt*0.6f;
        game.player2.charge = 0;
    } else if(key_down(GLFW_KEY_DOWN)) {
        game.player2.sprite.pos[1] -= dt*0.6f;
        game.player2.charge = 0;
    }

    // Outside board?
    game.player2.sprite.pos[1] = clamp(game.player2.sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void game_think(float dt)
{
    ball_think(dt);
    player1_think(dt);
    player2_think(dt);

    // Ball collides with Player 1?
    if(game.ball.vx < 0
            && game.ball.sprite.pos[1] >= (game.player1.sprite.pos[1] - PLAYER_HEIGHT/2 - BALL_HEIGHT/2)
            && game.ball.sprite.pos[1] <= (game.player1.sprite.pos[1] + PLAYER_HEIGHT/2 + BALL_HEIGHT/2)
            && game.ball.sprite.pos[0] <= (PLAYER1_HIT + PLAYER_WIDTH/2 + BALL_WIDTH/2)) {
        ball_player_bounce(&game.ball, &game.player1);
    }

    // Ball collides with Player 2?
    if(game.ball.vx > 0
            && game.ball.sprite.pos[1] >= (game.player2.sprite.pos[1] - PLAYER_HEIGHT/2 - BALL_HEIGHT/2)
            && game.ball.sprite.pos[1] <= (game.player2.sprite.pos[1] + PLAYER_HEIGHT/2 + BALL_HEIGHT/2)
            && game.ball.sprite.pos[0] >= (PLAYER2_HIT - PLAYER_WIDTH/2 - BALL_WIDTH/2)) {
        ball_player_bounce(&game.ball, &game.player2);
    }

    // Emit player charge particles
    think_player_charged(&game.player1, dt);
    think_player_charged(&game.player2, dt);
}

void particles_think(float dt)
{
    /* Think for each particle. */
    for(int i=0; i<game.particles_count; i++) {
        struct particle *p = &game.particles[i];

        p->age += dt;
        p->sprite.pos[0] += p->vx * dt;
        p->sprite.pos[1] += p->vy * dt;
        p->sprite.color[3] = PARTICLE_ALPHA - clamp(p->age / p->age_max, 0.0f, 1.0f) * PARTICLE_ALPHA;
        p->sprite.rotation += p->va * dt;

        if(p->age >= p->age_max) {
            p->dead = 1;
        }
    }

    /* Remove dead particles. */
    for(int i=0; i<game.particles_count; i++) {
        if(game.particles[i].dead) {
            for(int n=i+1; n<game.particles_count; n++) {
                game.particles[i] = game.particles[n];
            }
            game.particles_count --;
        }
    }
}

void shader_think(struct graphics *g, float delta_time)
{
    /* Upload uniforms. */
    glUniform1f(g->shader.uniforms[TIME], (GLfloat) game.time);

    /* Transform. */
    mat4 transform;
    mult(transform, g->translate, g->scale);
    mult(transform, transform, g->rotate);
    transpose_same(transform);
    glUniformMatrix4fv(g->shader.uniform_transform, 1, GL_FALSE, transform);

    /* Projection. */
    glUniformMatrix4fv(g->shader.uniform_projection, 1, GL_FALSE, g->projection);

    /* Ball stuff */
    glUniform1f(g->shader.uniforms[BALL_LAST_HIT], game.ball.last_hit);
}

void think(float delta_time)
{
    game_think(delta_time);
    particles_think(delta_time);
    shader_think(&game.graphics, delta_time);
    input_think(&game.input, delta_time);
}

void render(GLFWwindow *window, struct graphics *g)
{
    //glEnableClientState(0);
    glEnableVertexAttribArray(0);

    /* Clear. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(g->shader.program);

    /* Sprites. */
    glUniform1i(g->shader.uniforms[IS_BALL], 0);
    sprite_render(&game.player1.sprite, &game.graphics);
    sprite_render(&game.player2.sprite, &game.graphics);

    /* Ball. */
    glUniform1i(g->shader.uniforms[IS_BALL], 1);
    sprite_render(&game.ball.sprite, &game.graphics);

    /* Particles. */
    glUniform1i(g->shader.uniforms[IS_BALL], 0);
    for(int i=0; i<game.particles_count; i++) {
        if(!game.particles[i].dead) {
            sprite_render(&game.particles[i].sprite, &game.graphics);
        }
    }
}

void resize(GLFWwindow *window, int width, int height)
{
}

void init_player1(struct player *p)
{
    setv(p->sprite.pos, 32.0f, VIEW_HEIGHT/2, 0.0f, 1.0f);
    setv(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
    copyv(p->sprite.color, COLOR_WHITE);
}

void init_player2(struct player *p)
{
    setv(p->sprite.pos, 608.0f, VIEW_HEIGHT/2, 0.0f, 1.0f);
    setv(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
    copyv(p->sprite.color, COLOR_WHITE);
}

void init_ball(struct ball *ball)
{
    ball->speed = 0.6f;
    rand(); rand(); rand();
    float random_angle = randr(0.0f, 2.0f * M_PI);
    printf("random_angle=%6f\n", random_angle);
    ball->vy = ball->speed * sin(random_angle) / 4.0f;
    ball->vx = ball->speed * cos(random_angle) / 4.0f;
    ball->last_hit = 10.0f;

    setv(ball->sprite.pos, VIEW_WIDTH/2, VIEW_HEIGHT/2, 0.0f, 1.0f);
    setv(ball->sprite.scale, BALL_WIDTH, BALL_HEIGHT, 1.0f, 1.0f);
    copyv(ball->sprite.color, COLOR_WHITE);
}

void init()
{
    /* Graphics. */
    graphics_init(&game.graphics, VIEW_WIDTH, VIEW_HEIGHT, &vertex_shader,
            &fragment_shader, uniform_names, UNIFORM_LAST);

    /* Game setup. */
    game.time_mod = 1.0f;

    /* Entities. */
    init_player1(&game.player1);
    init_player2(&game.player2);
    init_ball(&game.ball);
}

void clean_up()
{
    graphics_free(&game.graphics);
    glfwTerminate();
}

GLFWwindow *window;

void do_frame()
{     
    /* Delta-time. */
    float delta_time = 0;
    if(game.time != 0) {
        delta_time = (glfwGetTime() - game.time) * 1000.0f * game.time_mod;
    }
    game.time = glfwGetTime();

    /* Game loop. */
    think(delta_time);
    render(window, &game.graphics);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();

    /* Register that a frame has been drawn. */
    graphics_count_frame(&game.graphics);
}

void key_callback(struct input *input, GLFWwindow *window, int key,
        int scancode, int action, int mods)
{
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
#ifndef EMSCRIPTEN
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, 1);
                break;
#endif
        }
    }
}

int main(int argc, char **argv)
{
    /* Seed random number generator. */
    srand(time(NULL));

    /* Initialize the library */
    if(!glfwInit()) {
        return -1;
    }

#ifdef EMSCRIPTEN
    window = glfwCreateWindow(VIEW_WIDTH, VIEW_HEIGHT, "glpong", NULL, NULL);
#else
    /* QUIRK: Mac OSX */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *pMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *pVideomode = glfwGetVideoMode(pMonitor); // TODO: Use to set resolution?

    /* Create a windowed mode window and its OpenGL context */
    if(argc >= 2 && strncmp(argv[1], "windowed", 8) == 0) {
        window = glfwCreateWindow(VIEW_WIDTH, VIEW_HEIGHT, "glpong", NULL, NULL);
    } else {
        window = glfwCreateWindow(pVideomode->width, pVideomode->height, "glpong", pMonitor, NULL);
    }

    if(!window) {
        glfwTerminate();
        return -1;
    }
#endif

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
    game.input.callback = key_callback;
    input_init(&game.input, window);
    glfwSetWindowSizeCallback(window, &resize);

    /* Loop until the user closes the window */
#ifdef EMSCRIPTEN
    emscripten_set_main_loop( do_frame, 0, 1 );
#else
    while(!glfwWindowShouldClose(window)) { 
        do_frame();
    }
#endif
    clean_up();
    return 0;
}
