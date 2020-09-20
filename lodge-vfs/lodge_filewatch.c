#include "lodge_filewatch.h"

#include "membuf.h"
#include "strbuf.h"

#include "lodge_assert.h"

#include <string.h>

#define LODGE_FILEWATCH_EVENT_QUEUE_MAX	256
#define LODGE_FILEWATCH_DIRS_MAX		256

#ifdef _WIN32

// NOTE: This Windows API is horrible.
//
// https://medium.com/tresorit-engineering/how-to-get-notifications-about-file-system-changes-on-windows-519dd8c4fb01
// http://old.zaynar.co.uk/cppdoc/latest/projects/lib.sysdep.win/wdir_watch.cpp.html

#include <stdio.h>
#include <windows.h>
#include <WinBase.h>

struct lodge_filewatch_entry
{
	size_t							index;

	uint32_t						refs;
	char							path[4096];

	HANDLE							dir_handle;

	char							change_buf[4096];
	DWORD							bytes_returned;
	OVERLAPPED						overlapped;
};

struct lodge_filewatch_event
{
	size_t							entry_index;
	char							path[4096];
	enum lodge_filewatch_reason		reason;
};

struct lodge_filewatch
{
	size_t							count;
	lodge_filewatch_func_t			funcs[LODGE_FILEWATCH_DIRS_MAX];
	void*							userdatas[LODGE_FILEWATCH_DIRS_MAX];

	struct lodge_filewatch_entry	entries[LODGE_FILEWATCH_DIRS_MAX];

	struct lodge_filewatch_event	event_queue[LODGE_FILEWATCH_EVENT_QUEUE_MAX];
	size_t							event_queue_count;

	HANDLE							io_completion_port_handle;
};

static enum lodge_filewatch_reason lodge_filewatch_action_to_reason(DWORD action)
{
	switch(action) 
	{
	case FILE_ACTION_ADDED:
		return LODGE_FILEWATCH_REASON_FILE_CREATED;
	case FILE_ACTION_REMOVED:
		return LODGE_FILEWATCH_REASON_FILE_DELETED;
	case FILE_ACTION_MODIFIED:
		return LODGE_FILEWATCH_REASON_FILE_MODIFIED;
	case FILE_ACTION_RENAMED_OLD_NAME:
	case FILE_ACTION_RENAMED_NEW_NAME:
	default:
		ASSERT_NOT_IMPLEMENTED();
		return LODGE_FILEWATCH_REASON_MAX;
	}
};

static void lodge_filewatch_queue_events(struct lodge_filewatch *filewatch, struct lodge_filewatch_entry *entry)
{
    ASSERT(entry);

    const char* pos = entry->change_buf;

    for(;;) {
        const FILE_NOTIFY_INFORMATION* file_notify_info = (const FILE_NOTIFY_INFORMATION*)pos;

		size_t event_index = filewatch->event_queue_count++;
		struct lodge_filewatch_event *event = &filewatch->event_queue[event_index];
		event->entry_index = entry->index;
		event->reason = lodge_filewatch_action_to_reason(file_notify_info->Action);
		//strbuf_setf(strbuf_wrap(event->path), "%s/%.*ls", watch->path, file_notify_info->FileNameLength, file_notify_info->FileName);
		strbuf_setf(strbuf_wrap(event->path), "%.*ls", file_notify_info->FileNameLength, file_notify_info->FileName);

        const DWORD offset = file_notify_info->NextEntryOffset;
        if(!offset) {
            break;
		}
        pos += offset;
    }
}

static void lodge_filewatch_poll(struct lodge_filewatch *filewatch)
{
    DWORD bytes_transferred;
    ULONG_PTR key;
    OVERLAPPED* overlapped;
    BOOL status_ret = GetQueuedCompletionStatus(filewatch->io_completion_port_handle, &bytes_transferred, &key, &overlapped, 0);
    if(!status_ret) {
        return;
	}

    const intptr_t entry_index = (intptr_t)key;
    struct lodge_filewatch_entry *entry = &filewatch->entries[entry_index];
    if(!entry || entry->refs == 0) {
        return;
	}

    if(bytes_transferred != 0) {
        lodge_filewatch_queue_events(filewatch, entry);
	}

    const DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                         FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
                         FILE_NOTIFY_CHANGE_CREATION;
    const DWORD buf_size = sizeof(entry->change_buf);
    memset(&entry->overlapped, 0, sizeof(entry->overlapped));
	
	// much faster than watching every dir separately. see dir_add_watch.
    BOOL ret = ReadDirectoryChangesW(entry->dir_handle, entry->change_buf, buf_size, TRUE, filter, &entry->bytes_returned, &entry->overlapped, 0);
    ASSERT(ret);
}

static void lodge_filewatch_broadcast_events(struct lodge_filewatch *filewatch)
{
	for(size_t i = 0; i < filewatch->event_queue_count; i++) {
		struct lodge_filewatch_event *event = &filewatch->event_queue[i];
		const size_t dir_index = event->entry_index;
		filewatch->funcs[dir_index](strbuf_wrap_and(event->path, strbuf_to_strview), event->reason, filewatch->userdatas[dir_index]);
	}

	filewatch->event_queue_count = 0;
}

struct lodge_filewatch* lodge_filewatch_new()
{
	struct lodge_filewatch *notify = (struct lodge_filewatch*) calloc(1, sizeof(struct lodge_filewatch));
	return notify;
}

void lodge_filewatch_free(struct lodge_filewatch *notify)
{
	for(size_t i = 0; i < notify->count; i++) {
		BOOL ret = CloseHandle(notify->entries[i].dir_handle);
		ASSERT(ret);
	}

	BOOL ret = CloseHandle(notify->io_completion_port_handle);
	ASSERT(ret);

	free(notify);
}

static void lodge_vfs_notify_add_dir_impl(struct lodge_filewatch *notify, size_t index, strview_t dir, bool recursive, lodge_filewatch_func_t func, void *func_userdata)
{
    {
	#if 0
		// check if this is a subdirectory of an already watched dir tree
		// (much faster than issuing a new watch for every subdir).
		// this also prevents watching the same directory twice.
		for(WatchIt it = watches->begin(); it != watches->end(); ++it)
		{
			Watch* const w = it->second;
			if(!w)
				continue;
			const char* old_dir = w->dir_name.c_str();
			if(path_is_subpath(dir, old_dir))
			{
				reqnum = w->reqnum;
				w->refs++;
				goto done;
			}
		}
	#endif

		// open handle to directory
		const DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		const DWORD flags = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;
		const HANDLE dir_handle = CreateFile(dir.s, FILE_LIST_DIRECTORY, share, 0, OPEN_EXISTING, flags, 0);
		if(dir_handle == INVALID_HANDLE_VALUE) {
			goto fail;
		}

		// associate Watch* with the directory handle. when we receive a packet
		// from the IOCP, we will need to re-issue the watch.
		const ULONG_PTR key = (ULONG_PTR)index;

		// create IOCP (if not already done) and attach hDir to it
		notify->io_completion_port_handle = CreateIoCompletionPort(dir_handle, notify->io_completion_port_handle, key, 0);
		if(notify->io_completion_port_handle == 0 || notify->io_completion_port_handle == INVALID_HANDLE_VALUE) {
			CloseHandle(dir_handle);
			goto fail;
		}

		membuf_set(membuf_wrap(notify->entries), index, &(struct lodge_filewatch_entry) {
			.refs = 1,
			.index = index,
			.dir_handle = dir_handle,
		}, sizeof(struct lodge_filewatch_entry));

		strbuf_set(strbuf_wrap(notify->entries[index].path), dir);

		// post a dummy kickoff packet; the IOCP polling code will "re"issue
		// the corresponding watch. this keeps the ReadDirectoryChangesW call
		// and directory <--> Watch association code in one place.
		//
		// we call get_packet so that it's issued immediately,
		// instead of only at the next call to dir_get_changed_file.
		PostQueuedCompletionStatus(notify->io_completion_port_handle, 0, key, 0);
		lodge_filewatch_poll(notify);
    }

done:
	return;

fail:
	ASSERT_FAIL("Failed to watch dir");
}

void lodge_filewatch_add_dir(struct lodge_filewatch *filewatch, strview_t dir, bool recursive, lodge_filewatch_func_t func, void *func_userdata)
{
	membuf_append(membuf_wrap(filewatch->funcs), &func, sizeof(lodge_filewatch_func_t), &filewatch->count);
	membuf_set(membuf_wrap(filewatch->userdatas), filewatch->count-1, &func_userdata, sizeof(void*));

	lodge_vfs_notify_add_dir_impl(filewatch, filewatch->count-1, dir, recursive, func, func_userdata);
}

void lodge_filewatch_update(struct lodge_filewatch *filewatch, float dt)
{
	// Poll for notifications quickly and add them in the events queue.
	lodge_filewatch_poll(filewatch);
	// Broadcast events now that listeners are restarted (these may take a while).
	lodge_filewatch_broadcast_events(filewatch);
}

#else

struct lodge_filewatch* lodge_filewatch_new()
{
	ASSERT_NOT_IMPLEMENTED();
	return NULL;
}

void lodge_filewatch_free(struct lodge_filewatch *filewatch)
{
	ASSERT_NOT_IMPLEMENTED();
}

void lodge_filewatch_update(struct lodge_filewatch *filewatch, float dt)
{
	ASSERT_NOT_IMPLEMENTED();
}

void lodge_filewatch_add_dir(struct lodge_filewatch *filewatch, strview_t dir, bool recursive, lodge_filewatch_func_t func, void *func_userdata)
{
	ASSERT_NOT_IMPLEMENTED();
}

#endif