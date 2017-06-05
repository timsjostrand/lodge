/**
 * OpenAL wrapper.
 *
 * Unless otherwise noted sound data is 16 bit mono.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stb/stb_vorbis.c>
#define AL_LIBTYPE_STATIC
#ifndef EMSCRIPTEN
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "sound.h"
#include "alist.h"
#include "math4.h"

static size_t sound_buf_read_file(stb_vorbis *header, stb_vorbis_info *info,
		ALshort *buf, int len);
static struct sound_emitter* sound_emitter_get(struct sound *s);

static int al_test(const char *msg)
{
	ALCenum error = alGetError();
	if(error != AL_NO_ERROR) {
		sound_error("%s [%d]\n", msg, error);
		return SOUND_ERROR;
	}
	return SOUND_OK;
}

static ALenum to_al_format(short channels, short samples)
{
	int stereo = (channels == 2);

	switch(samples) {
	case 16:
		return stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	case 8:
		return stereo ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
	default:
		return -1;
	}
}

static void sound_list_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	sound_debug("Devices list:\n");
	sound_debug("----------\n");
	while(device && *device != '\0' && next && *next != '\0') {
		sound_debug("%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	sound_debug("----------\n");
}

/**
 * @param s				The sound context to initialize into.
 * @param listener_pos	The position where the listener is. For a 2D game, this
 *						might be the center of the screen. For a 3D game, this
 *						might be the camera position.
 * @param max_distance	The maximum distance from the listener where a sound is
 *						still audible.
 */
int sound_init(struct sound *s, vec3 listener_pos, float max_distance)
{
	/* List devices if extension is available. */
	ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if(enumeration == AL_FALSE) {
		sound_error("enumeration extension not available\n");
	} else {
		sound_list_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
	}

	/* Open the default device. */
	const ALCchar *defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice *device = alcOpenDevice(defaultDeviceName);
	if(!device) {
		sound_error("unable to open default device\n");
		return SOUND_ERROR;
	}

	/* Dump the opened device to stdout. */
	sound_debug("Device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
	alGetError();

	/* Create the context for the audio scene. */
	s->context = alcCreateContext(device, NULL);
	if(!alcMakeContextCurrent(s->context)) {
		sound_error("failed to make default context\n");
		return SOUND_ERROR;
	}
	AL_TEST("make default context");

	/* Set up 2D listener. */
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	AL_TEST("distance model");
	alListener3f(AL_POSITION, xyz(listener_pos));
	AL_TEST("listener position");
	alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	AL_TEST("listener velocity");
#if 0
	alListenerfv(AL_ORIENTATION, listenerOri);
	AL_TEST("listener orientation");
#endif

	/* Set up sound object. */
	s->emitters = alist_new(SOUND_EMITTERS_MAX);

	/* Generate static sound sources. */
	sound_src_t sources[SOUND_EMITTERS_MAX];
	alGenSources(SOUND_EMITTERS_MAX, sources);
	AL_TEST("generate sources");

	/* Assign each emitter a unique source name. */
	for(int i=0; i<SOUND_EMITTERS_MAX; i++) {
		sound_emitter_init(&(s->emitters_mem[i]));
		s->emitters_mem[i].src = sources[i];
		alSourcef(sources[i], AL_MAX_DISTANCE, max_distance);
		alSourcef(sources[i], AL_ROLLOFF_FACTOR, 1);
	}

	return SOUND_OK;
}

static void sound_src_free_all(struct sound *s)
{
	for(int i=0; i<SOUND_EMITTERS_MAX; i++) {
		struct sound_emitter *em = &(s->emitters_mem[i]);
		//sound_emitter_stop(em);
		if(alIsSource(em->src)) {
			alDeleteSources(1, &em->src);
		}
	}
}

void sound_free(struct sound *s)
{
	/* Free sources. */
	sound_src_free_all(s);
	/* Free OpenAL resources. */
	ALCdevice *device = alcGetContextsDevice(s->context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(s->context);
	alcCloseDevice(device);
	/* Free local memory. */
	alist_free(s->emitters, 0);
}

/**
 * Play a sound buffer with control over all the playback parameters.
 *
 * @param s					The sound context to play the sound in.
 * @param buf				The OpenAL pre-uploaded buffer to play.
 * @param pos				The 3D position vector of the sound emitter.
 * @param velocity			The 3D velocity vector of the sound emitter.
 * @param loop				Whether to loop the buffer or not.
 * @param gain				Gain/volume to play this buffer with (default 1.0).
 * @param pitch				Pitch factor to play this buffer with (default 1.0).
 * @param uninterruptable	Set to true if this sound cannot be interrupted due
 *							to the sound pool running out. Use only for certain
 *							sound effects, like music or important dialog.
 *
 * @return					A pointer to the sound emitter assigned to the
 *							buffer, or NULL on error.
 */
struct sound_emitter* sound_buf_play_detailed(struct sound *s, const sound_buf_t buf,
		float *pos, float *velocity, ALboolean loop, float gain, float pitch,
		int uninterruptable)
{
	/* Get available emitter. */
	struct sound_emitter *em = sound_emitter_get(s);
	if(em == NULL) {
		sound_debug("Could not get new emitter\n");
		return NULL;
	}

	/* Bind buffer to source. */
	alSourcei(em->src, AL_BUFFER, buf);
	if(al_test("bind buffer") != SOUND_OK) {
		return NULL;
	}

	/* Put emitter last in queue to be interrupted. */
	alist_append(s->emitters, em);

	/* Set attributes. */
	sound_src_loop(em->src, loop);
	sound_src_pitch(em->src, pitch);
	sound_src_gain(em->src, gain);

	/* Set up emitter. */
	em->pos = pos;
	em->velocity = velocity;
	em->uninterruptable = uninterruptable;
	sound_emitter_update(em);

	/* Start playing source. */
	alSourcePlay(em->src);
	al_test("play source");

	return em;
}

/**
 * Start playing an uninterruptable, looping music track. Use sound_src_stop()
 * to stop it.
 */
struct sound_emitter* sound_buf_play_music(struct sound *s, const sound_buf_t buf,
		float gain)
{
	return sound_buf_play_detailed(s, buf, NULL, NULL, AL_TRUE, gain, 1.0f, 1);
}

/**
 * Plays a non-looping sound buffer with a random pitch variation between
 * [1.0-variation, 1.0+variation].
 */
struct sound_emitter* sound_buf_play_pitched(struct sound *s, const sound_buf_t buf,
		float *pos, float pitch_range)
{
	return sound_buf_play_detailed(s, buf, pos, NULL, AL_FALSE, 1.0f,
			randr(1.0f - pitch_range, 1.0f + pitch_range), 0);
}

/**
 * Plays a non-looping sound with zero velocity and default pitch and gain.
 */
struct sound_emitter* sound_buf_play(struct sound *s, const sound_buf_t buf,
		float *pos)
{
	return sound_buf_play_detailed(s, buf, pos, NULL, AL_FALSE, 1.0f, 1.0f, 0);
}

/**
 * Loads a 16 bit mono PCM buffer.
 */
int sound_buf_load_pcm(sound_buf_t *buf, ALshort *data, size_t len)
{
	ALenum format = to_al_format(1, SOUND_SAMPLE_SIZE);

	/* Generate PCM buffers. */
	alGenBuffers(1, buf);
	AL_TEST("sound_buf_load_pcm: generate buffer");

	/* Bind buffer. */
	alBufferData((*buf),
			format,
			data,
			len,
			SOUND_SAMPLE_RATE);
	AL_TEST("buffer data");

	return SOUND_OK;
}

int sound_buf_load_vorbis_header(sound_buf_t *buf, stb_vorbis *header)
{
	/* File info. */
	stb_vorbis_info info = stb_vorbis_get_info(header);
	ALenum format = to_al_format(info.channels, 16);

	/* Load file. */
	unsigned int samples_count = stb_vorbis_stream_length_in_samples(header) * info.channels;
	size_t data_len = samples_count * sizeof(ALshort);
	ALshort *data = (ALshort *) malloc(data_len);
	if(data == NULL) {
		sound_error("Out of memory\n");
		return SOUND_ERROR;
	}
	size_t size = sound_buf_read_file(header, &info, data, data_len);
	if(size <= 0) {
		return SOUND_ERROR;
	}

	/* Generate PCM buffers. */
	alGenBuffers(1, (ALuint *) buf);
	AL_TEST("sound_buf_load_vorbis_header: generate buffer");

	/* Bind buffer. */
	alBufferData((*buf), format, data, size*sizeof(ALshort), info.sample_rate);
	AL_TEST("buffer data");

	return SOUND_OK;
}

int sound_src_pitch(sound_src_t src, float pitch)
{
	alSourcef(src, AL_PITCH, pitch);
	AL_TEST("source pitch");
	return SOUND_OK;
}

int sound_src_gain(sound_src_t src, float gain)
{
	alSourcef(src, AL_GAIN, gain);
	AL_TEST("source gain");
	return SOUND_OK;
}

int sound_src_position(sound_src_t src, float x, float y, float z)
{
	alSource3f(src, AL_POSITION, x, y, z);
	AL_TEST("source position");
	return SOUND_OK;
}

int sound_src_velocity(sound_src_t src, float vx, float vy, float vz)
{
	alSource3f(src, AL_VELOCITY, vx, vy, vz);
	AL_TEST("source velocity");
	return SOUND_OK;
}

int sound_src_loop(sound_src_t src, ALboolean loop)
{
	alSourcei(src, AL_LOOPING, loop);
	AL_TEST("source looping");
	return SOUND_OK;
}

static size_t sound_buf_read_file(stb_vorbis *header, stb_vorbis_info *info,
		ALshort *buf, int len)
{
	size_t size = 0;
	int read = 0;

	stb_vorbis_seek_start(header);

	/* Fill the buffer. */
	while(size < len) {
		read = stb_vorbis_get_samples_short_interleaved(header, info->channels,
				buf + size, len - size);
		if(read > 0) {
			size += read * info->channels;
		} else {
			break;
		}
	}

	/* End of file. */
	if(size == 0) {
		sound_debug("end of file\n");
	}

	return size;
}

int sound_buf_load_vorbis(sound_buf_t *buf, const void *data, size_t len)
{
	stb_vorbis *vorbis = stb_vorbis_open_memory((const unsigned char *) data, len, NULL, NULL);
	if(!vorbis) {
		sound_error("Could not parse vorbis data\n");
		return SOUND_ERROR;
	}

	int ret = sound_buf_load_vorbis_header(buf, vorbis);

	stb_vorbis_close(vorbis);

	return ret;
}

int sound_buf_load_vorbis_file(sound_buf_t *buf, const char *filename)
{
	stb_vorbis *vorbis = stb_vorbis_open_filename(filename, NULL, NULL);
	if(!vorbis) {
		sound_error("Could not open vorbis file\n");
		return SOUND_ERROR;
	}

	int ret = sound_buf_load_vorbis_header(buf, vorbis);

	stb_vorbis_close(vorbis);

	return ret;
}

/**
 * Adds a mono tone with the specified frequency into a buffer.
 *
 * @param freq		The frequency of the tone in Hz.
 * @param buf		The buffer to add into.
 * @param offset	The number of samples that have already passed.
 * @param len		The number of samples to add into the buffer.
 *
 * @return			Number of samples written to the buffer.
 */
static int sound_filter_add_tone(double freq, ALshort *buf, size_t offset,
		size_t len)
{
	double theta = (freq * 2 * M_PI) / (double) SOUND_SAMPLE_RATE;
	for(int i=0; i < len; i++) {
		buf[i] += (ALshort) (SOUND_SAMPLE_MAX * sin(theta * (double) (i + offset)));
	}
	return len;
}

int sound_filter_add_440hz(ALshort *buf, size_t offset, size_t len)
{
	return sound_filter_add_tone(440.0, buf, offset, len);
}

int sound_filter_add_220hz(ALshort *buf, size_t offset, size_t len)
{
	return sound_filter_add_tone(220.0, buf, offset, len);
}

int sound_filter_half_gain(ALshort *buf, size_t offset, size_t len)
{
	ALshort *end = buf + len;
	for(; buf<end; buf++) {
		*buf >>= 1;
	}
	return len;
}

/**
 * Takes a buffer and adds a filter to it.
 *
 * @param buf		The buffer to filter.
 * @param start		The offset in the buffer to start at in samples.
 * @param len		The length of the buffer in samples.
 * @param filter	The filter to apply.
 */
size_t sound_buf_add_filter(ALshort *buf, const size_t start, const size_t len,
		filter_t filter)
{
	size_t size = start;

	/* Fill the buffer. */
	while(size < len) {
		size_t read = filter(buf + size, size, len - size);
		if(read > 0) {
			size += read;
		} else {
			break;
		}
	}

	/* End of file. */
	if(size == 0) {
		sound_debug("sound_buf_add_filter(): 0 samples added\n");
	}

	return size;
}

int sound_buf_load_filter(sound_buf_t *buf, size_t samples_count,
		const int sample_rate, filter_t filter)
{
	ALenum format = to_al_format(1, SOUND_SAMPLE_SIZE);

	/* Load data. */
	ALshort *data = (ALshort *) malloc(samples_count * sizeof(ALshort));
	if(data == NULL) {
		sound_error("Out of memory\n");
		return SOUND_ERROR;
	}

	/* Fill buffer. */
	memset(data, 0, samples_count * sizeof(ALshort));
	sound_buf_add_filter(data, 0, samples_count, filter);

	/* Generate PCM buffers. */
	alGenBuffers(1, buf);
	AL_TEST("sound_buf_load_filter: generate buffer");

	/* Bind buffer. */
	alBufferData((*buf),
			format,
			data,
			samples_count * sizeof(ALshort),
			sample_rate);
	AL_TEST("buffer data");
	free(data);

	return SOUND_OK;
}

int sound_src_stop(sound_src_t src)
{
	ALint source_state;
	alGetSourcei(src, AL_SOURCE_STATE, &source_state);

	if(source_state == AL_PLAYING) {
		alSourceStop(src);
		AL_TEST("source stop");
	}

	return SOUND_OK;
}

void sound_buf_free(sound_buf_t buf)
{
	if(alIsBuffer(buf)) {
		alDeleteBuffers(1, &buf);
	}
}

void sound_master_gain(float gain)
{
	alListenerf(AL_GAIN, gain);
}

void sound_emitter_init(struct sound_emitter *em)
{
	em->available = 1;
	em->pos = NULL;
	em->velocity = NULL;
}

/**
 * Make an emitter ready for use.
 */
void sound_emitter_stop(struct sound_emitter *em)
{
	sound_src_stop(em->src);
	sound_emitter_init(em);
}

struct sound_emitter* sound_emitter_get_next_interruptable(struct sound *s)
{
	foreach_alist_p(struct sound_emitter *, em, s->emitters) {
		if(!(*em)->uninterruptable) {
			return (*em);
		}
	}
	return NULL;
}

/**
 * Get the next available sound emitter, or force one to be available.
 */
static struct sound_emitter* sound_emitter_get(struct sound *s)
{
	/* Any available emitters? */
	for(int i=0; i<SOUND_EMITTERS_MAX; i++) {
		if(s->emitters_mem[i].available) {
			s->emitters_mem[i].available = 0;
			return &(s->emitters_mem[i]);
		}
	}
	/* No available emitters: force stop oldest emitter. */
	struct sound_emitter *emitter = sound_emitter_get_next_interruptable(s);
	/* Sanity check. */
	if(emitter == NULL) {
		sound_error("All emitters busy and non-interruptable\n");
		return NULL;
	}
	sound_debug("All emitters busy: terminating %u...\n", emitter->src);
	sound_emitter_stop(emitter);
	emitter->available = 0;
	return emitter;
}

static void sound_emitters_clean(struct sound *s)
{
	ALint source_state;
	foreach_alist(struct sound_emitter *, em, index, s->emitters) {
		alGetSourcei(em->src, AL_SOURCE_STATE, &source_state);
		if(source_state != AL_PLAYING) {
			/* Make available. */
			em->available = 1;
			/* Remove emitter. */
			alist_delete_at(s->emitters, index, 0);
			/* Deleted self: stay on same index for next element. */
			index--;
		}
	}
}

void sound_emitter_update(struct sound_emitter *em)
{
	if(em->pos != NULL) {
		sound_src_position(em->src, xyz(em->pos));
	}
	if(em->velocity != NULL) {
		sound_src_velocity(em->src, xyz(em->velocity));
	}
}

void sound_think(struct sound *s, float delta_time)
{
	/* Remove done emitters. */
	sound_emitters_clean(s);

	/* Update emitter attributes. */
	foreach_alist_p(struct sound_emitter *, emitter, s->emitters) {
		sound_emitter_update((*emitter));
	}
}
