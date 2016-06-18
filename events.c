#include "events.h"
#include "math4.h"
#include "log.h"

static void events_noop_callback(void *data, unsigned int data_len)
{
	errorf("Events", "NOOP event with length %u\n", data_len);
}

void events_init(struct events* events)
{
	events->events_count = 0;
	events->registered_count = 0;

	/* Reserve event ID 0 */
	events_register(events, "noop", &events_noop_callback);
}

void events_send(struct events* events, const struct event* event)
{
	struct event *new_event = &events->events[events->events_count];
	memcpy(new_event, event, sizeof(struct event));
	events->events_count++;
}

void events_update(struct events* events)
{
	for (int i = 0; i < events->events_count; i++)
	{
		struct event* event = &events->events[i];
		events->registered[event->id].callback(event->data, event->data_length);
	}

	events->events_count = 0;
}

unsigned int events_register(struct events *events, char *name, events_handle_callback_t callback)
{
	struct events_registered_event *reg = &events->registered[events->registered_count];
	strncpy(reg->name, name, EVENTS_NAME_MAX);
	reg->callback = callback;
	reg->id = events->registered_count;
	return events->registered_count++;
}
