#ifndef _SOUND_H
#define _SOUND_H

#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#ifdef DEBUG
#define sound_debug(...) fprintf(stderr, "AL_DEBUG: " __VA_ARGS__)
#else
#define sound_debug(...) do {} while (0)
#endif

#define sound_error(...) fprintf(stderr, "AL_ERROR: " __VA_ARGS__)

#define SOUND_OK            0
#define SOUND_ERROR         -1

#define AL_TEST(msg) if(al_test(msg) != SOUND_OK) { return SOUND_ERROR; }

struct sound {
    ALCcontext  *context;
};

struct sound_fx {
    ALuint      source;
    ALuint      buffer;
};

int     sound_fx_open(struct sound_fx *s, const char *filename);
void    sound_fx_free(struct sound_fx *s);
int     sound_fx_play(struct sound_fx *s);
int     sound_fx_stop(struct sound_fx *s);
int     sound_fx_pitch(struct sound_fx *s, float pitch);
int     sound_fx_gain(struct sound_fx *s, float gain);
int     sound_fx_position(struct sound_fx *s, float x, float y, float z);
int     sound_fx_velocity(struct sound_fx *s, float vx, float vy, float vz);
int     sound_fx_loop(struct sound_fx *s, ALboolean loop);
size_t  sound_fx_read_file(stb_vorbis *header, stb_vorbis_info *info,
        ALshort *buf, int len);

int     sound_init(struct sound *s);
void    sound_free(struct sound *s);

#endif
