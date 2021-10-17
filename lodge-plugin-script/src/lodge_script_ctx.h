#ifndef _LODGE_SCRIPT_CTX_H
#define _LODGE_SCRIPT_CTX_H

struct lodge_scene;

struct lodge_script_component;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_script_ctx
{
	struct lodge_scene				*scene;
	lodge_entity_t					entity;
	struct lodge_script_component	*component;
};

#endif