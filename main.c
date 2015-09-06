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

#include "math4.h"
#include "color.h"
#include "graphics.h"
#include "input.h"
#include "texture.h"
#include "sound.c"

#define VFS_ENABLE_FILEWATCH
#include "vfs.h"

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

#define SPRITE_TYPE_UNKNOWN     0
#define SPRITE_TYPE_BALL        1
#define SPRITE_TYPE_PLAYER      2
#define SPRITE_TYPE_PARTICLE    3

enum uniforms {
    TIME,
    BALL_LAST_HIT_X,
    BALL_LAST_HIT_Y,
    UNIFORM_LAST
};

const char *uniform_names[] = {
    "time",
    "ball_last_hit_x",
    "ball_last_hit_y",
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
    float           last_hit_x;
    float           last_hit_y;
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

struct textures {
    GLuint  none;
    GLuint  test;
};

struct game {
    double          time;                       /* Time since start. */
    struct graphics graphics;                   /* Graphics state. */
    struct player   player1;                    /* Left player. */
    struct player   player2;                    /* Right player. */
    struct ball     ball;
    struct stats    total_stats;
    struct particle particles[PARTICLES_MAX];
    int             particles_count;
    struct input    input;
    struct textures textures;
    struct sound    sound;
    struct sound_fx vivaldi;
    struct sound_fx tone_hit;
    struct sound_fx tone_bounce;
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
    "in vec2 texcoord;"
    "out vec4 frag_color;"
    "uniform float time;"
    "uniform vec4 color;"
    "uniform sampler2D tex;"
    "void main() {"
    "   frag_color = texture(tex, texcoord) * color;"
    "}";

const char *vertex_shader =
    "#version 400 core\n"
    "precision highp float;"
    ""
    "const int TYPE_UNKNOWN     = 0;"
    "const int TYPE_BALL        = 1;"
    "const int TYPE_PLAYER      = 2;"
    "const int TYPE_PARTICLE    = 3;"
    ""
    "uniform mat4 transform;"
    "uniform mat4 projection;"
    "uniform float time;"
    "uniform int sprite_type;"
    "uniform float ball_last_hit_x;"
    "uniform float ball_last_hit_y;"
    ""
    "const vec4 wobble_x_offsets[6] = vec4[6]("
    "   vec4( 0.25,  0.5, 0.0, 0.0),"
    "   vec4( 0.25, -0.5, 0.0, 0.0),"
    "   vec4(-0.25,  0.5, 0.0, 0.0),"
    "   vec4(-0.25,  0.5, 0.0, 0.0),"
    "   vec4( 0.25, -0.5, 0.0, 0.0),"
    "   vec4(-0.25, -0.5, 0.0, 0.0)"
    ");"
    ""
    "const vec4 wobble_y_offsets[6] = vec4[6]("
    "   vec4(-0.25, -0.25, 0.0, 0.0),"
    "   vec4(-0.25,  0.25, 0.0, 0.0),"
    "   vec4( 0.25, -0.25, 0.0, 0.0),"
    "   vec4( 0.25, -0.25, 0.0, 0.0),"
    "   vec4(-0.25,  0.25, 0.0, 0.0),"
    "   vec4( 0.25,  0.25, 0.0, 0.0)"
    ");"
    ""
    "in vec3 vp;"
    "in vec2 texcoord_in;"
    "out vec2 texcoord;"
    "void main() {"
    "   texcoord = texcoord_in;"
    "   vec4 offset = (ball_last_hit_x < ball_last_hit_y) ? wobble_x_offsets[gl_VertexID] : wobble_y_offsets[gl_VertexID];"
    "   float bh = min(ball_last_hit_x, ball_last_hit_y);"
    "   if(bh <= 0.016)"
    "       bh = 0.016;"
    "   float decay = 0.016 / (bh*0.0001);"
    "   decay = clamp(decay, 0.0, 1.5);"
    "   float hit_wobble = abs(cos(bh/80.0)) * decay;"
    "   gl_Position = projection * transform * ("
    "       vec4(vp, 1.0)"
    "       + offset*float(sprite_type == TYPE_BALL)*hit_wobble"
    "       + 0.0*offset*float(sprite_type == TYPE_BALL)*cos(time*8.0f)/6.0"
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
    p->sprite.type = SPRITE_TYPE_PARTICLE;
    p->sprite.rotation = angle;
    p->sprite.texture = game.textures.none;

    set4f(p->sprite.pos, x, y, 0.0f, 1.0f);
    set4f(p->sprite.color, rgb(COLOR_WHITE), PARTICLE_ALPHA);
    set4f(p->sprite.scale, w, h, 1.0f, 1.0f);
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
    ball->last_hit_x = 0.0f;

    p->charge = 0.0f;

    sound_fx_play(&game.tone_hit);
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
        game.ball.last_hit_y = 0.0f;
        sound_fx_play(&game.tone_bounce);
    } else if(game.ball.sprite.pos[1] < BOARD_BOTTOM + BALL_HEIGHT/2) {
        game.ball.sprite.pos[1] = BOARD_BOTTOM + BALL_HEIGHT/2;
        game.ball.vy *= -1.0f;
        game.ball.last_hit_y = 0.0f;
        sound_fx_play(&game.tone_bounce);
    }

    // Ball: move
    game.ball.sprite.pos[0] += dt*game.ball.vx;
    game.ball.sprite.pos[1] += dt*game.ball.vy;
    game.ball.last_hit_x += dt;
    game.ball.last_hit_y += dt;
}

void player1_think(struct player *p, float dt)
{
    p->charge += dt;

    // Move?
    if(key_down(GLFW_KEY_W)) {
        p->sprite.pos[1] += dt*0.6f;
        p->charge = 0;
    } else if(key_down(GLFW_KEY_S)) {
        p->sprite.pos[1] -= dt*0.6f;
        p->charge = 0;
    }

    // Outside board?
    p->sprite.pos[1] = clamp(p->sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void player2_think(struct player *p, float dt)
{
    p->charge += dt;

    // Move?
    if(key_down(GLFW_KEY_UP)) {
        p->sprite.pos[1] += dt*0.6f;
        p->charge = 0;
    } else if(key_down(GLFW_KEY_DOWN)) {
        p->sprite.pos[1] -= dt*0.6f;
        p->charge = 0;
    }

    // Outside board?
    p->sprite.pos[1] = clamp(p->sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void game_think(float dt)
{
    ball_think(dt);
    player1_think(&game.player1, dt);
    player2_think(&game.player2, dt);

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

    /* Ball stuff */
    glUniform1f(g->shader.uniforms[BALL_LAST_HIT_X], game.ball.last_hit_x);
    glUniform1f(g->shader.uniforms[BALL_LAST_HIT_Y], game.ball.last_hit_y);
}

void think(struct graphics *g, float delta_time)
{
	VFS_FILEWATCH

    game.time = glfwGetTime();
    game_think(delta_time);
    particles_think(delta_time);
    shader_think(g, delta_time);
    input_think(&game.input, delta_time);
}

void render(struct graphics *g, float delta_time)
{
    //glEnableClientState(0);
    glEnableVertexAttribArray(0);

    /* Clear. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(g->shader.program);

    /* Sprites. */
    sprite_render(&game.player1.sprite, &game.graphics);
    sprite_render(&game.player2.sprite, &game.graphics);

    /* Ball. */
    sprite_render(&game.ball.sprite, &game.graphics);

    /* Particles. */
    for(int i=0; i<game.particles_count; i++) {
        if(!game.particles[i].dead) {
            sprite_render(&game.particles[i].sprite, &game.graphics);
        }
    }
}

void init_player1(struct player *p)
{
    p->sprite.type = SPRITE_TYPE_PLAYER;
    p->sprite.texture = game.textures.none;
    set4f(p->sprite.pos, 32.0f, VIEW_HEIGHT/2, 0.0f, 1.0f);
    set4f(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
    copyv(p->sprite.color, COLOR_WHITE);
}

void init_player2(struct player *p)
{
    p->sprite.type = SPRITE_TYPE_PLAYER;
    p->sprite.texture = game.textures.none;
    set4f(p->sprite.pos, 608.0f, VIEW_HEIGHT/2, 0.0f, 1.0f);
    set4f(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
    copyv(p->sprite.color, COLOR_WHITE);
}

void init_ball(struct ball *ball)
{
    ball->sprite.type = SPRITE_TYPE_BALL;
    ball->sprite.texture = game.textures.test;
    ball->speed = 0.6f;
    rand(); rand(); rand();
    float random_angle = randr(0.0f, 2.0f * M_PI);
    printf("random_angle=%6f\n", random_angle);
    ball->vy = ball->speed * sin(random_angle) / 4.0f;
    ball->vx = ball->speed * cos(random_angle) / 4.0f;
    ball->last_hit_x = 10.0f;
    ball->last_hit_y = 0.0f;

    set4f(ball->sprite.pos, VIEW_WIDTH/2, VIEW_HEIGHT/2, 0.0f, 1.0f);
    set4f(ball->sprite.scale, BALL_WIDTH, BALL_HEIGHT, 1.0f, 1.0f);
    copyv(ball->sprite.color, COLOR_WHITE);
}

void init_game()
{
    /* Entities. */
    init_player1(&game.player1);
    init_player2(&game.player2);
    init_ball(&game.ball);
}

void key_callback(struct input *input, GLFWwindow *window, int key,
        int scancode, int action, int mods)
{
    if(action == GLFW_RELEASE) {
        switch(key) {
            case GLFW_KEY_O:
                game.graphics.delta_time_factor *= 2.0f;
                sound_fx_pitch(&game.vivaldi, game.graphics.delta_time_factor);
                printf("time_mod=%f\n", game.graphics.delta_time_factor);
                break;
            case GLFW_KEY_P:
                game.graphics.delta_time_factor /= 2.0f;
                sound_fx_pitch(&game.vivaldi, game.graphics.delta_time_factor);
                printf("time_mod=%f\n", game.graphics.delta_time_factor);
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, 1);
                break;
        }
    }
}

void test_make_sound_manual()
{
    /* 1 s buffer. */
    ALshort buf[SOUND_SAMPLE_RATE] = { 0 };

    /* Add a 440 Hz tone into the buffer. */
    sound_fx_add_filter(buf, 0, SOUND_SAMPLE_RATE, sound_filter_add_440hz);

    /* Lower the volume of half the buffer. */
    sound_fx_add_filter(buf, SOUND_SAMPLE_RATE/2, SOUND_SAMPLE_RATE, sound_filter_half_gain);

    /* Make a sound. */
    if(sound_fx_load_pcm(&game.tone_hit, buf, sizeof(buf)) != SOUND_OK) {
        sound_error("Could not load manual sound\n");
    }
}

void load_sounds()
{
    if(sound_fx_load_vorbis_file(&game.vivaldi, "vivaldi.ogg") != SOUND_OK) {
        sound_error("sound_stream_open\n");
    } else {
        sound_fx_play(&game.vivaldi);
    }

    if(sound_fx_load_filter(&game.tone_hit,
                0.1 * SOUND_SAMPLE_RATE,
                SOUND_SAMPLE_RATE,
                &sound_filter_add_440hz) != SOUND_OK) {
        sound_error("could not generate tone\n");
    }

    if(sound_fx_load_filter(&game.tone_bounce,
                0.1 * SOUND_SAMPLE_RATE,
                SOUND_SAMPLE_RATE,
                &sound_filter_add_220hz) != SOUND_OK) {
        sound_error("could not generate tone\n");
    }
}

void release_sounds()
{
    sound_fx_free(&game.vivaldi);
}

void release_textures()
{
    texture_free(game.textures.none);
    texture_free(game.textures.test);
}

void load_textures()
{
    /* Create a square white texture for texturing empty sprites with. */
    texture_white(&game.textures.none);

    /* Load textures. */
    int ret = texture_load(&game.textures.test, "test.png");
    if(ret != GRAPHICS_OK) {
        graphics_error("Texture load failed\n");
    }
}

void test_read_file(const char* filename, unsigned int size, void* data)
{
	printf("%s: ", filename);
	for (unsigned int i = 0; i < size; i++)
	{
		printf("%c", ((char*)data)[i]);
	}
	printf("\n\n");
}

void reload_vertex_shader(const char* filename, unsigned int size, void* data)
{
	vfs_free_memory(filename);
}

void reload_fragment_shader(const char* filename, unsigned int size, void* data)
{
	vfs_free_memory(filename);
}

int main(int argc, char **argv)
{
	/* Start the virtual file system */
	vfs_init();

	/* Asset location */
	vfs_mount("C:/Users/Johan/Desktop/assets");
	
	/* Register asset callbacks */
	vfs_register_callback("basic_shader.frag", reload_fragment_shader);
	vfs_register_callback("basic_shader.vert", reload_vertex_shader);

    int ret = 0;

    /* Seed random number generator. */
    srand(time(NULL));

    /* Set up sound. */
    sound_init(&game.sound);

    /* Set up graphics. */
    int windowed = (argc >= 2 && strncmp(argv[1], "windowed", 8) == 0);
    ret = graphics_init(&game.graphics, &think, &render, VIEW_WIDTH, VIEW_HEIGHT,
            windowed, &vertex_shader, &fragment_shader, uniform_names,
            UNIFORM_LAST);

    if(ret != GRAPHICS_OK) {
        graphics_error("Graphics initialization failed (%d)\n", ret);
        graphics_free(&game.graphics);
        exit(ret);
    }

    /* Load assets. */
    load_sounds();
    load_textures();

    /* Get input events. */
    game.input.callback = key_callback;
    ret = input_init(&game.input, game.graphics.window);

    if(ret != GRAPHICS_OK) {
        graphics_error("Input initialization failed (%d)\n", ret);
        graphics_free(&game.graphics);
        exit(ret);
    }

    /* Set up game. */
    init_game();

	/* Load all assets */
	vfs_run_callbacks();

    /* Loop until the user closes the window */
    graphics_loop(&game.graphics);

    /* Release assets. */
    release_sounds();
    release_textures();

    /* Free OpenAL. */
    sound_free(&game.sound);

    /* If we reach here, quit the game. */
    graphics_free(&game.graphics);

	vfs_shutdown();

    return 0;
}
