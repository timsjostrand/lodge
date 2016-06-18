#include "events_sound.h"
#include "sound.h"

static unsigned int events_sound_id_play = 0;

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

void events_sound_send_play(struct events *events, struct sound *sound,
	const sound_buf_t buf, vec3 pos, vec3 velocity, ALboolean loop, float gain,
	float pitch, int uninterruptable)
{
	struct event event = { 0 };
	event.id = events_sound_id_play;
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

static void events_sound_handle_play(void *data, unsigned int data_len)
{
	struct events_data_playsound *d = (struct events_data_playsound *) data;
	sound_buf_play_detailed(d->sound, d->buf, d->pos, d->velocity, d->loop, d->gain, d->pitch, d->uninterruptable);
}

void events_sound_register(struct events *events)
{
	events_sound_id_play = events_register(events, "sound_play", &events_sound_handle_play);
}