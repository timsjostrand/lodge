#ifndef _EVENTS_H
#define _EVENTS_H

#include "sound.h"

#define EVENTS_DATA_MAX 1024
#define EVENTS_MAX 256

enum event_type {
	EVENT_PLAYSOUND = 0
};

struct event {
	char data[EVENTS_DATA_MAX];
	enum event_type type;
	int data_length;
};

struct events{
	struct event events[EVENTS_MAX];
	int events_count;
};

void events_init(struct events* events);
void events_send(struct events* events, struct event* event);
void events_update(struct events* events);

/* Helpers. */

void events_send_playsound(struct events *events, struct sound *sound,
	const sound_buf_t buf, vec3 pos, vec3 velocity, ALboolean loop, float gain,
	float pitch, int uninterruptable);

#endif // _EVENTS_H
