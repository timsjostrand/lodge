#include "lodge_log.h"

#include "lodge_platform.h"

#include "strview.h"
#include "membuf.h"
#include "dynbuf.h"
#include "strbuf.h"

#include <stdarg.h>
#include <stdio.h>

struct lodge_log_desc
{
	lodge_log_func_t		func;
	void					*userdata;
};

struct lodge_log_entry
{
	enum lodge_log_level	level;
	strview_t				src;
	char					message[1024];
	size_t					message_length;
};

struct lodge_log_entries
{
	struct lodge_log_entry	*elements;
	size_t					count;
	size_t					capacity;
};

struct lodge_log_desc		log_descs[32];
size_t						log_descs_count = 0;
struct lodge_log_entries	log_entries;

void lodge_log_add_func(lodge_log_func_t func, void *userdata)
{
	membuf_append(
		membuf(log_descs),
		&log_descs_count,
		&(struct lodge_log_desc) {
			.func = func,
			.userdata = userdata
		},
		sizeof(struct lodge_log_desc)
	);

	if(log_entries.count > 0) {
		for(size_t i = 0; i < log_entries.count; i++) {
		struct lodge_log_entry *entry = &log_entries.elements[i];
			func(entry->level, entry->src, strview_make(entry->message, entry->message_length), userdata);
		}
		dynbuf_clear(dynbuf(log_entries));
	}
}

void lodge_log_remove_func(lodge_log_func_t func, void *userdata)
{
	size_t index = membuf_find(
		membuf(log_descs),
		log_descs_count,
		&(struct lodge_log_desc) {
			.func = func,
			.userdata = userdata
		},
		sizeof(struct lodge_log_desc)
	);

	ASSERT_OR(index >= 0) { return; }

	membuf_delete(membuf(log_descs), &log_descs_count, index, 1);
}

void lodge_log(enum lodge_log_level level, strview_t src, strview_t message)
{
	if(log_descs_count > 0) {
		for(size_t i = 0; i < log_descs_count; i++) {
			log_descs[i].func(level, src, message, log_descs[i].userdata);
		}
	}
	else {
		//
		// `log_entries` is meant to buffer the earliest log messages, before a log
		// consumer has been registered. This assert is to sanity check that we
		// don't append too many entries.
		// 
		// Long-term solution to this is to use a circular buffer instead, so that
		// the last N messages can be consumed when a log func is registered.
		//
		ASSERT(log_entries.count < 256);

		//
		// TODO(TS): circular append
		//
		struct lodge_log_entry* log_entry = dynbuf_append_no_init(dynbuf(log_entries));

		log_entry->level = level;
		log_entry->src = src;
		strbuf_set(strbuf(log_entry->message), message);
		log_entry->message_length = message.length;
	}
}

void lodge_logf(enum lodge_log_level level, strview_t src, strview_t fmt, ...)
{
	static char buf[1024];
	va_list args;
	va_start(args, fmt);
	size_t len = vsnprintf(buf, lodge_countof(buf), fmt.s, args);
	va_end(args);
	lodge_log(level, src, strview_make(buf, len));
}