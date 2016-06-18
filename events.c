#include "events.h"
#include "math4.h"

static struct events_data_playsound {
	struct sound *sound;
	sound_buf_t buf;
	vec3 pos;
	vec3 velocity;
	ALboolean loop;
	float gain;
	float pitch;
	int uninterruptable;
};

void events_init(struct events* events)
{
	events->events_count = 0;
}

void events_send(struct events* events, const struct event* event)
{
	struct event *new_event = &events->events[events->events_count];
	memcpy(new_event, event, sizeof(struct event));
	events->events_count++;
}

void events_send_playsound(struct events *events, struct sound *sound,
	const sound_buf_t buf, vec3 pos, vec3 velocity, ALboolean loop, float gain,
	float pitch, int uninterruptable)
{
	struct event event = { 0 };
	event.type = EVENT_PLAYSOUND;
	event.data_length = sizeof(struct events_data_playsound);

	struct events_data_playsound *sound_data = (struct events_data_playsound *) event.data;
	sound_data->sound = sound;
	sound_data->buf = buf;
	set3f(sound_data->pos, xyz(pos));
	set3f(sound_data->velocity, xyz(velocity));
	sound_data->loop = loop;
	sound_data->gain = gain;
	sound_data->pitch = pitch;
	sound_data->uninterruptable = uninterruptable;

	events_send(events, &event);
}


static void event_handle_playsound(void *data)
{
	struct events_data_playsound *d = (struct events_data_playsound *) data;
	sound_buf_play_detailed(d->sound, d->buf, d->pos, d->velocity, d->loop, d->gain, d->pitch, d->uninterruptable);
}

void events_update(struct events* events)
{
	for (int i = 0; i < events->events_count; i++)
	{
		struct event* event = &events->events[i];

		switch (event->type)
		{
		case EVENT_PLAYSOUND:
			event_handle_playsound(event->data);
			break;
		}
	}

	events->events_count = 0;
}
