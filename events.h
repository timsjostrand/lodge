#ifndef _EVENTS_H
#define _EVENTS_H

#define EVENTS_DATA_MAX 1024
#define EVENTS_MAX 256
#define EVENTS_NAME_MAX 256
#define EVENTS_REGISTER_MAX 2048

struct event {
	char data[EVENTS_DATA_MAX];
	unsigned int id;
	unsigned int data_length;
};

typedef void(*events_handle_callback_t)(void *data, unsigned int data_len);

struct events_registered_event {
	char						name[EVENTS_NAME_MAX];
	unsigned int				id;
	events_handle_callback_t	callback;
};

struct events {
	struct event events[EVENTS_MAX];
	struct events_registered_event registered[EVENTS_REGISTER_MAX];
	int events_count;
	int registered_count;
};

void events_init(struct events* events);
void events_send(struct events* events, struct event* event);
void events_update(struct events* events);
unsigned int events_register(struct events *events, char *name, events_handle_callback_t callback);

#endif // _EVENTS_H
