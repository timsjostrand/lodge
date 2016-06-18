#ifndef _EVENTS_SOUND_H
#define _EVENTS_SOUND_H

#include "events.h"

void events_sound_send_play(struct events *events, struct sound *sound,
	const sound_buf_t buf, vec3 pos, vec3 velocity, ALboolean loop, float gain,
	float pitch, int uninterruptable);

void events_sound_register(struct events *events);

#endif