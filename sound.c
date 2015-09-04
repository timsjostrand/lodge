/**
 * OpenAL wrapper.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdbool.h>
#include <stb/stb_vorbis.c>
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>

#include "sound.h"

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
    bool stereo = (channels > 1);

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

    /* set orientation */
#if 0
    alListener3f(AL_POSITION, 0, 0, 1.0f);
    AL_TEST("listener position");
    alListener3f(AL_VELOCITY, 0, 0, 0);
    AL_TEST("listener velocity");
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
    sound_debug("source_state=%d\n", source_state);

    if(source_state != AL_PLAYING) {
        alSourcePlay(s->source);
        AL_TEST("could not play source");
    }

    return SOUND_OK;
}

int sound_fx_open_header(struct sound_fx *s, stb_vorbis *header)
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

size_t sound_fx_read_file(stb_vorbis *header, stb_vorbis_info *info,
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

int sound_fx_open(struct sound_fx *s, const char *filename)
{
    memset(s, 0, sizeof(struct sound_fx));

    stb_vorbis *vorbis = stb_vorbis_open_filename(filename, NULL, NULL);
    if(!vorbis) {
        sound_error("Could not open vorbis file\n");
        return SOUND_ERROR;
    }

    int ret = sound_fx_open_header(s, vorbis);

    stb_vorbis_close(vorbis);

    return ret;
}

int sound_fx_stop(struct sound_fx *s)
{
    alSourceStop(s->source);
    AL_TEST("source stop");
    return SOUND_OK;
}

void sound_fx_free(struct sound_fx *s)
{
    sound_fx_stop(s);
    alDeleteSources(1, &s->source);
    alDeleteBuffers(1, &s->buffer);
}
