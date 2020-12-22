#ifndef _LODGE_SCENE_SERIALIZE_H
#define _LODGE_SCENE_SERIALIZE_H

#include "strview.h"

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

char*			lodge_scene_to_text(lodge_scene_t scene, size_t *size_out);
lodge_scene_t	lodge_scene_from_text(strview_t text);

char*			lodge_scene_entity_to_text(lodge_scene_t scene, const lodge_entity_t entity, size_t *size_out);
lodge_entity_t	lodge_scene_entity_from_text(lodge_scene_t scene, strview_t text);

#endif