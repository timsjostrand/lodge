#ifndef _LODGE_PLUGIN_SHADER_SOURCES_H
#define _LODGE_PLUGIN_SHADER_SOURCES_H

#include "lodge_plugin.h"
#include "strview.h"

struct lodge_shader_source;

strview_t					lodge_shader_source_get_source(struct lodge_shader_source *source);

LODGE_PLUGIN_DECL(lodge_plugin_shader_sources);

#endif
