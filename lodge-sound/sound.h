#ifndef _SOUND_H
#define _SOUND_H

#include <limits.h>

#include "log.h"
#include "alist.h"
#include "math4.h"

#define SOUND_OK			0
#define SOUND_ERROR			-1

#define SOUND_SAMPLE_SIZE	16
#define SOUND_SAMPLE_RATE	44100
#define SOUND_SAMPLE_MAX	SHRT_MAX
#define SOUND_EMITTERS_MAX	32

#define sound_debug(...) debugf("Sound", __VA_ARGS__)
#define sound_error(...) errorf("Sound", __VA_ARGS__)

#define AL_TEST(msg) if(al_test(msg) != SOUND_OK) { return SOUND_ERROR; }

typedef unsigned int sound_buf_t;
typedef unsigned int sound_src_t;
typedef short ALshort;
typedef char ALboolean;

struct sound_emitter {
	sound_src_t		src;				/* OpenAL Source Name assigned to this emitter. */
	int				available;			/* Whether this emitter is available to play sounds. */
	int				uninterruptable;	/* If this sound is uninterruptable until it is done playing (for music, dialog, etc.) */
	vec3			*pos;				/* X,Y,Z components in space (or NULL). */
	vec3			*velocity;			/* X,Y,Z velocity components (or NULL). */
};

struct sound {
	void    				*context;							/* OpenAL context. */
	struct alist			*emitters;							/* Emitters in ascending order of creation time (0 is oldest). */
	struct sound_emitter	emitters_mem[SOUND_EMITTERS_MAX];	/* Static memory used to hold emitter data. */
};

typedef int (*filter_t)(ALshort *buf, size_t offset, size_t len);

int		sound_buf_load_vorbis(sound_buf_t *buf, const void *data, size_t len);
int		sound_buf_load_vorbis_file(sound_buf_t *buf, const char *filename);
int		sound_buf_load_filter(sound_buf_t *buf, size_t samples_count,
				const int sample_rate, filter_t filter);
int		sound_buf_load_pcm(sound_buf_t *buf, ALshort *data, size_t len);
void	sound_buf_free(sound_buf_t buf);
size_t	sound_buf_add_filter(ALshort *buf, const size_t start, const size_t len,
				filter_t filter);

struct sound_emitter* sound_buf_play_detailed(struct sound *s, const sound_buf_t buf,
				vec3 *pos, vec3 *velocity, ALboolean loop, float gain, float pitch,
				int uninterruptable);
struct sound_emitter* sound_buf_play_pitched(struct sound *s, const sound_buf_t buf,
				vec3 *pos, float pitch_range);
struct sound_emitter* sound_buf_play_music(struct sound *s, const sound_buf_t buf,
				float gain);
struct sound_emitter* sound_buf_play(struct sound *s, const sound_buf_t buf, vec3 *pos);

int		sound_src_stop(sound_src_t src);
int		sound_src_pitch(sound_buf_t buf, float pitch);
int		sound_src_gain(sound_buf_t buf, float gain);
int		sound_src_position(sound_buf_t buf, float x, float y, float z);
int		sound_src_velocity(sound_buf_t buf, float vx, float vy, float vz);
int		sound_src_loop(sound_buf_t buf, ALboolean loop);

void	sound_emitter_init(struct sound_emitter *em);
void	sound_emitter_stop(struct sound_emitter *em);
void	sound_emitter_update(struct sound_emitter *em);

int		sound_init(struct sound *s, vec3 listener_pos, float max_distance);
void	sound_free(struct sound *s);
void	sound_think(struct sound *s, float delta_time);
void	sound_master_gain(float gain);

int		sound_filter_add_440hz(ALshort *buf, size_t offset, size_t len);
int		sound_filter_add_220hz(ALshort *buf, size_t offset, size_t len);
int		sound_filter_half_gain(ALshort *buf, size_t offset, size_t len);

#endif
