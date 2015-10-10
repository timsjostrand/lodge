#ifndef _SOUND_H
#define _SOUND_H

#ifndef EMSCRIPTEN
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <limits.h>
#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#include "log.h"

#define SOUND_OK			0
#define SOUND_ERROR			-1

#define SOUND_SAMPLE_SIZE	16
#define SOUND_SAMPLE_RATE	44100
#define SOUND_SAMPLE_MAX	SHRT_MAX

#define sound_debug(...) debugf("Sound", __VA_ARGS__)
#define sound_error(...) errorf("Sound", __VA_ARGS__)

#define AL_TEST(msg) if(al_test(msg) != SOUND_OK) { return SOUND_ERROR; }

struct sound {
	ALCcontext	*context;
};

struct sound_fx {
	ALuint		source;
	ALuint		buffer;
};

typedef int (*filter_t)(ALshort *buf, size_t offset, size_t len);

int		sound_fx_load_vorbis(struct sound_fx *s, const void *buf, size_t len);
int		sound_fx_load_vorbis_file(struct sound_fx *s, const char *filename);
int		sound_fx_load_filter(struct sound_fx *s, size_t samples_count,
				const int sample_rate, filter_t filter);
int		sound_fx_load_pcm(struct sound_fx *s, ALshort *buf, size_t len);
void	sound_fx_free(struct sound_fx *s);
size_t	sound_fx_add_filter(ALshort *buf, const size_t start, const size_t len,
		filter_t filter);
int		sound_fx_play(struct sound_fx *s);
int		sound_fx_stop(struct sound_fx *s);
int		sound_fx_pitch(struct sound_fx *s, float pitch);
int		sound_fx_gain(struct sound_fx *s, float gain);
int		sound_fx_position(struct sound_fx *s, float x, float y, float z);
int		sound_fx_velocity(struct sound_fx *s, float vx, float vy, float vz);
int		sound_fx_loop(struct sound_fx *s, ALboolean loop);

int		sound_init(struct sound *s);
void	sound_free(struct sound *s);
void	sound_master_gain(float gain);

int		sound_filter_add_440hz(ALshort *buf, size_t offset, size_t len);
int		sound_filter_add_220hz(ALshort *buf, size_t offset, size_t len);
int		sound_filter_half_gain(ALshort *buf, size_t offset, size_t len);

#endif
