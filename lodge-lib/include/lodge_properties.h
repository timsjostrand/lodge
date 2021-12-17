#ifndef _LODGE_PROPERTIES_H
#define _LODGE_PROPERTIES_H

#include "lodge_type.h"
#include "lodge_platform.h"

#include <stdbool.h>

struct lodge_property;

enum lodge_property_flag
{
	//
	// Default flags.
	//
	LODGE_PROPERTY_FLAG_NONE		= 0,
	
	//
	// Do not show in editor.
	//
	LODGE_PROPERTY_FLAG_PRIVATE		= LODGE_BIT(1),

	//
	// Not editable in editor.
	//
	LODGE_PROPERTY_FLAG_READ_ONLY	= LODGE_BIT(2),

	//
	// Transient properties are not serialized.
	//
	LODGE_PROPERTY_FLAG_TRANSIENT	= LODGE_BIT(3),

	//
	// This property can only be written to, which is useful when a property is
	// representing multiple underlying properties with different values.
	//
	LODGE_PROPERTY_FLAG_WRITE_ONLY	= LODGE_BIT(4),
};

typedef bool							(*lodge_property_pre_modified_func_t)(struct lodge_property *property, const void *object, const void *dst, const void *src);
typedef void							(*lodge_property_modified_func_t)(struct lodge_property *property, void *object);

struct lodge_property_hints
{
	bool								enable;
	union
	{
		struct
		{
			float						min;
			float						max;
			float						step;
		}								f32;
		struct
		{
			uint32_t					min;
			uint32_t					max;
			uint32_t					step;
		}								u32;
		struct
		{
			int32_t						min;
			int32_t						max;
			int32_t						step;
		}								i32;
		struct 
		{
			bool						color;
		}								vec4;
		void							*custom;
	};
};

struct lodge_property
{
	strview_t							name;
	lodge_type_t						type;
	size_t								offset;
	enum lodge_property_flag			flags;
	lodge_property_pre_modified_func_t	pre_modified;
	lodge_property_modified_func_t		on_modified;
	void								*userdata;
	struct lodge_property_hints			hints;
};

struct lodge_properties
{
	size_t								count;
	struct lodge_property				elements[255];
};

const void*								lodge_property_get(const struct lodge_property *property, const void *object);
void									lodge_property_set(struct lodge_property *property, void *object, const void *src);
bool									lodge_property_meta_equals(struct lodge_property *lhs, struct lodge_property *rhs);

#endif