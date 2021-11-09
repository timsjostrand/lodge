#ifndef _LODGE_NS_EDITOR_H
#define _LODGE_NS_EDITOR_H

#include <stddef.h>

struct lodge_gui;
typedef struct lodge_gui* lodge_gui_t;

struct lodge_graph;
typedef struct lodge_graph* lodge_graph_t;

struct lodge_ns_editor;

struct lodge_ns_editor*		lodge_ns_editor_new(lodge_graph_t graph);
void						lodge_ns_editor_free(struct lodge_ns_editor *editor);

void						lodge_ns_editor_new_inplace(struct lodge_ns_editor *editor, lodge_graph_t graph);
void						lodge_ns_editor_free_inplace(struct lodge_ns_editor *editor);
size_t						lodge_ns_editor_sizeof();

void						lodge_ns_editor_update(struct lodge_ns_editor *editor, lodge_gui_t gui, float dt);

#endif