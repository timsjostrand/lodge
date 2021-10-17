#ifndef _LODGE_EDITOR_PANEL
#define _LODGE_EDITOR_PANEL

struct lodge_editor_panel_type_desc
{
	strview_t			name;
	size_t				size;
	void				(*new_in_place)(void *panel, vec2 pos, vec2 size);
	void				(*free_in_place)(void *panel);
};

struct lodge_editor_panel;
typedef struct lodge_editor_panel* lodge_editor_panel_t;

lodge_editor_panel_type_t	lodge_editor_panel_type_register(struct lodge_editor_panel_type_desc *desc);
void						lodge_editor_panel_type_unregister(lodge_editor_panel_type_t panel_type);

lodge_editor_panel_t		lodge_editor_panel_open(lodge_editor_panel_type_t panel_type);
void						lodge_editor_panel_close(lodge_editor_panel_t panel);

#endif