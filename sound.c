/**
 * OpenAL wrapper.
 *
 * Unless otherwise noted sound data is 16 bit mono.
 *
 * TODO:
 *
 * - Play sources in "channels": CHAN_MUSIC, CHAN_EFFECTS and so on. These should
 *	 probably be pooled. So sounds playing in CHAN_MUSIC can never be
 *	 interrupted, but CHAN_EFFECTS may be stopped if a new sound should be
 *	 played and there are no more available resources.
 *
 * - Separate buffers from channels. Load a file (buffer) and play it later (source).
 *
 * - How many channels can be used? Allocate channels on play() or for every
 *	 entity? Or on-demand when sound_play() is called?
 *
 * - Add sound_{pitch,gain}() that sets pitch and gain globally for all sounds.
 *	 So for instance, if time_delta_factor is changed all sounds are slowed
 *	 down/up. This will however require all sounds to be registered somewhere
 *	 (will be required for pooling resources anyways?), which they are not
 *	 currently.
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

static size_t sound_fx_read_file(stb_vorbis *header, stb_vorbis_info *info,
		ALshort *buf, int len);

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

int sound_init(struct sound *s)
{
	ALboolean enumeration;
	const ALCchar *defaultDeviceName;
	ALCdevice *device;

	/* List devices if extension is available. */
	enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if(enumeration == AL_FALSE) {
		sound_error("enumeration extension not available\n");
	} else {
		sound_list_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
	}

	/* Open the default device. */
	defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	device = alcOpenDevice(defaultDeviceName);
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
	alListener3f(AL_POSITION, 0, 0, 0.0f);
	AL_TEST("listener position");
	alListener3f(AL_VELOCITY, 0, 0, 0);
	AL_TEST("listener velocity");
#if 0
	alListenerfv(AL_ORIENTATION, listenerOri);
	AL_TEST("listener orientation");
#endif

	return SOUND_OK;
}

void sound_free(struct sound *s)
{
	ALCdevice *device = alcGetContextsDevice(s->context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(s->context);
	alcCloseDevice(device);
}

int sound_fx_play(struct sound_fx *s)
{
	ALint source_state;
	alGetSourcei(s->source, AL_SOURCE_STATE, &source_state);

	if(source_state != AL_PLAYING) {
		alSourcePlay(s->source);
		AL_TEST("could not play source");
	}

	return SOUND_OK;
}

/**
 * Loads a 16 bit mono PCM buffer.
 */
int sound_fx_load_pcm(struct sound_fx *s, ALshort *buf, size_t len)
{
	ALenum format = to_al_format(1, SOUND_SAMPLE_SIZE);

	/* Generate the audio source (emitter). */
	alGenSources(1, &s->source);
	AL_TEST("generate source");

	/* Generate PCM buffers. */
	alGenBuffers(1, &s->buffer);
	AL_TEST("generate buffer");

	/* Bind buffer. */
	alBufferData(s->buffer,
			format,
			buf,
			len,
			SOUND_SAMPLE_RATE);
	AL_TEST("buffer data");

	/* Bind buffer to source. */
	alSourcei(s->source, AL_BUFFER, s->buffer);
	AL_TEST("bind buffer");

	sound_fx_pitch(s, 1.0f);
	sound_fx_gain(s, 1.0f);
	sound_fx_position(s, 0, 0, 0);
	sound_fx_velocity(s, 0, 0, 0);
	sound_fx_loop(s, AL_FALSE);

	return SOUND_OK;
}

int sound_fx_load_vorbis_header(struct sound_fx *s, stb_vorbis *header)
{
	/* File info. */
	stb_vorbis_info info = stb_vorbis_get_info(header);
	ALenum format = to_al_format(info.channels, 16);

	/* Load file. */
	unsigned int samples_count = stb_vorbis_stream_length_in_samples(header) * info.channels;
	size_t buf_len = samples_count * sizeof(ALshort);
	ALshort *buf = (ALshort *) malloc(buf_len);
	if(buf == NULL) {
		sound_error("Out of memory\n");
		return SOUND_ERROR;
	}
	size_t size = sound_fx_read_file(header, &info, buf, buf_len);
	if(size <= 0) {
		return SOUND_ERROR;
	}

	/* Generate the audio source (emitter). */
	alGenSources(1, &s->source);
	AL_TEST("generate source");

	/* Generate PCM buffers. */
	alGenBuffers(1, &s->buffer);
	AL_TEST("generate buffer");

	/* Bind buffer. */
	alBufferData(s->buffer, format, buf, size*sizeof(ALshort), info.sample_rate);
	AL_TEST("buffer data");

	/* Bind buffer to source. */
	alSourcei(s->source, AL_BUFFER, s->buffer);
	AL_TEST("bind buffer");

	sound_fx_pitch(s, 1.0f);
	sound_fx_gain(s, 1.0f);
	sound_fx_position(s, 0, 0, 0);
	sound_fx_velocity(s, 0, 0, 0);
	sound_fx_loop(s, AL_FALSE);

	return SOUND_OK;
}

int sound_fx_pitch(struct sound_fx *s, float pitch)
{
	alSourcef(s->source, AL_PITCH, pitch);
	AL_TEST("source pitch");
	return SOUND_OK;
}

int sound_fx_gain(struct sound_fx *s, float gain)
{
	alSourcef(s->source, AL_GAIN, gain);
	AL_TEST("source gain");
	return SOUND_OK;
}

int sound_fx_position(struct sound_fx *s, float x, float y, float z)
{
	alSource3f(s->source, AL_POSITION, x, y, z);
	AL_TEST("source position");
	return SOUND_OK;
}

int sound_fx_velocity(struct sound_fx *s, float vx, float vy, float vz)
{
	alSource3f(s->source, AL_VELOCITY, vx, vy, vz);
	AL_TEST("source velocity");
	return SOUND_OK;
}

int sound_fx_loop(struct sound_fx *s, ALboolean loop)
{
	alSourcei(s->source, AL_LOOPING, loop);
	AL_TEST("source looping");
	return SOUND_OK;
}

static size_t sound_fx_read_file(stb_vorbis *header, stb_vorbis_info *info,
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

int sound_fx_load_vorbis(struct sound_fx *s, const void *buf, size_t len)
{
	memset(s, 0, sizeof(struct sound_fx));

	stb_vorbis *vorbis = stb_vorbis_open_memory((const unsigned char *) buf, len, NULL, NULL);
	if(!vorbis) {
		sound_error("Could not parse vorbis data\n");
		return SOUND_ERROR;
	}

	int ret = sound_fx_load_vorbis_header(s, vorbis);

	stb_vorbis_close(vorbis);

	return ret;
}

int sound_fx_load_vorbis_file(struct sound_fx *s, const char *filename)
{
	memset(s, 0, sizeof(struct sound_fx));

	stb_vorbis *vorbis = stb_vorbis_open_filename(filename, NULL, NULL);
	if(!vorbis) {
		sound_error("Could not open vorbis file\n");
		return SOUND_ERROR;
	}

	int ret = sound_fx_load_vorbis_header(s, vorbis);

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
size_t sound_fx_add_filter(ALshort *buf, const size_t start, const size_t len,
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
		sound_debug("sound_fx_add_filter(): 0 samples added\n");
	}

	return size;
}

int sound_fx_load_filter(struct sound_fx *s, size_t samples_count,
		const int sample_rate, filter_t filter)
{
	ALenum format = to_al_format(1, SOUND_SAMPLE_SIZE);

	/* Load data. */
	ALshort *buf = (ALshort *) malloc(samples_count * sizeof(ALshort));
	if(buf == NULL) {
		sound_error("Out of memory\n");
		return SOUND_ERROR;
	}

	/* Fill buffer. */
	memset(buf, 0, samples_count * sizeof(ALshort));
	sound_fx_add_filter(buf, 0, samples_count, filter);

	/* Generate the audio source (emitter). */
	alGenSources(1, &s->source);
	AL_TEST("generate source");

	/* Generate PCM buffers. */
	alGenBuffers(1, &s->buffer);
	AL_TEST("generate buffer");

	/* Bind buffer. */
	alBufferData(s->buffer,
			format,
			buf,
			samples_count * sizeof(ALshort),
			sample_rate);
	AL_TEST("buffer data");
	free(buf);

	/* Bind buffer to source. */
	alSourcei(s->source, AL_BUFFER, s->buffer);
	AL_TEST("bind buffer");

	sound_fx_pitch(s, 1.0f);
	sound_fx_gain(s, 1.0f);
	sound_fx_position(s, 0, 0, 0);
	sound_fx_velocity(s, 0, 0, 0);
	sound_fx_loop(s, AL_FALSE);

	return SOUND_OK;
}

int sound_fx_stop(struct sound_fx *s)
{
	ALint source_state;
	alGetSourcei(s->source, AL_SOURCE_STATE, &source_state);

	if(source_state == AL_PLAYING) {
		alSourceStop(s->source);
		AL_TEST("source stop");
	}

	return SOUND_OK;
}

void sound_fx_free(struct sound_fx *s)
{
	if(alIsSource(s->source)) {
		sound_fx_stop(s);
		alDeleteSources(1, &s->source);
	}
	if(alIsBuffer(s->buffer)) {
		alDeleteBuffers(1, &s->buffer);
	}
}

void sound_master_gain(float gain)
{
	alListenerf(AL_GAIN, gain);
}
