#ifndef _LODGE_PLUGIN_RENDERDOC_H
#define _LODGE_PLUGIN_RENDERDOC_H

#include "lodge_plugin.h"
#include <stdbool.h>

struct lodge_plugin_renderdoc;

void						lodge_plugin_renderdoc_start_frame_capture(struct lodge_plugin_renderdoc *plugin);
bool						lodge_plugin_renderdoc_stop_frame_capture(struct lodge_plugin_renderdoc *plugin);

LODGE_PLUGIN_DECL(lodge_plugin_renderdoc);

#endif
