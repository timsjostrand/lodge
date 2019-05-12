#include "fbx.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <stb/stb_image.h> // for stbi_zlib_decode_buffer()

#include "lodge_assert.h"
#include "str.h"
#include "blob.h"
#include "alist.h"

#define FBX_FILE_MAGIC "Kaydara FBX Binary  "
#define FBX_NULL_NODE_SIZE 13

struct fbx_array_type
{
	uint32_t				array_length;
	uint32_t				encoding;
	uint32_t				compressed_length;
	const char*				contents;
};

enum fbx_property_type
{
	FBX_PROPERTY_TYPE_INT16			= 'Y',
	FBX_PROPERTY_TYPE_BOOL			= 'C',
	FBX_PROPERTY_TYPE_INT32			= 'I',
	FBX_PROPERTY_TYPE_FLOAT			= 'F',
	FBX_PROPERTY_TYPE_DOUBLE		= 'D',
	FBX_PROPERTY_TYPE_INT64			= 'L',
	FBX_PROPERTY_TYPE_BINARY		= 'R',
	FBX_PROPERTY_TYPE_STRING		= 'S',
	FBX_PROPERTY_TYPE_ARRAY_FLOAT	= 'f',
	FBX_PROPERTY_TYPE_ARRAY_INT32	= 'i',
	FBX_PROPERTY_TYPE_ARRAY_DOUBLE	= 'd',
	FBX_PROPERTY_TYPE_ARRAY_INT64	= 'l',
	FBX_PROPERTY_TYPE_ARRAY_BOOL	= 'b',
	FBX_PROPERTY_TYPE_ARRAY_CHAR	= 'c',
};

struct fbx_property_special
{
	uint32_t				length;
	char					data[];
};

struct fbx_property_array
{
	uint32_t				array_length;
	uint32_t				encoding;
	uint32_t				compressed_length;
	char					data[];
};

struct fbx_property
{
	char					type;
	void*					heap_data;
	char					static_data[sizeof(double)];
};

struct fbx_node
{
	uint32_t				end_offset;
	uint32_t				properties_count;
	uint32_t				properties_list_len;
	uint8_t					name_len;
	char					name[255];
	struct alist			*properties;
	struct alist			*children;
};

struct fbx
{
	char					magic[21];	// Bytes 0 - 20: `Kaydara FBX Binary  \x00` (file - magic, with 2 spaces at the end, then a NULL terminator).
	char					pad[2];		// Bytes 21 - 22: [0x1A, 0x00] (unknown but all observed files show these bytes).
	uint32_t				version;	// Bytes 23 - 26: unsigned int, the version number. 7300 for version 7.3 for example.
	struct alist			*children;
};

static void fbx_print_header(const struct fbx *fbx)
{
	printf("magic: %s\n",		fbx->magic);
	printf("pad: 0x%x 0x%x\n",	fbx->pad[0], fbx->pad[1]);
	printf("version: %u\n",		fbx->version);
}

static void fbx_print_node(const struct fbx_node *node, int indent);

static void fbx_print_node_list(const struct alist *list, int indent)
{
	foreach_alist_p(struct fbx_node*, it, list) {
		fbx_print_node(*it, indent);
	}
}

static const char* fbx_property_get_type_string(const char type)
{
	switch(type)
	{
	case FBX_PROPERTY_TYPE_INT16:
		return "int16";
	case FBX_PROPERTY_TYPE_BOOL:
		return "bool";
	case FBX_PROPERTY_TYPE_INT32:
		return "int32";
	case FBX_PROPERTY_TYPE_FLOAT:
		return "float";
	case FBX_PROPERTY_TYPE_DOUBLE:
		return "double";
	case FBX_PROPERTY_TYPE_INT64:
		return "int64";
	case FBX_PROPERTY_TYPE_BINARY:
		return "binary";
	case FBX_PROPERTY_TYPE_STRING:
		return "string";
	case FBX_PROPERTY_TYPE_ARRAY_FLOAT:
		return "array_float";
	case FBX_PROPERTY_TYPE_ARRAY_INT32:
		return "array_int32";
	case FBX_PROPERTY_TYPE_ARRAY_DOUBLE:
		return "array_double";
	case FBX_PROPERTY_TYPE_ARRAY_INT64:
		return "array_int64";
	case FBX_PROPERTY_TYPE_ARRAY_BOOL:
		return "array_bool";
	case FBX_PROPERTY_TYPE_ARRAY_CHAR:
		return "array_char";
	default:
		ASSERT_FAIL("FBX: Unknown property type");
		return "unknown";
	}
}

static void fbx_print_property_list(const struct alist *list, int indent)
{
	foreach_alist_p(struct fbx_property*, it, list) {
		printf("%*sproperty: %s ", indent * 4, "", fbx_property_get_type_string((*it)->type));

		switch((*it)->type)
		{
		case FBX_PROPERTY_TYPE_INT16:
			printf("(%" PRId16 ")", *(const int16_t*)(*it)->static_data);
			break;
		case FBX_PROPERTY_TYPE_BOOL:
			printf("(%s)", (*it)->static_data[0] ? "true" : "false" );
			break;
		case FBX_PROPERTY_TYPE_INT32:
			printf("(%" PRId32 ")", *(const int32_t*)(*it)->static_data);
			break;
		case FBX_PROPERTY_TYPE_FLOAT:
			printf("(%f)", *((const float*)(*it)->static_data));
			break;
		case FBX_PROPERTY_TYPE_DOUBLE:
			printf("(%f)", *((const double*)(*it)->static_data));
			break;
		case FBX_PROPERTY_TYPE_INT64:
			printf("(%" PRId64 ")", *((const int64_t*)(*it)->static_data));
			break;
		case FBX_PROPERTY_TYPE_BINARY:
		case FBX_PROPERTY_TYPE_STRING:
		{
			ASSERT((*it)->heap_data);
			const struct fbx_property_special *prop_special = (const struct fbx_property_special*)(*it)->heap_data;
			printf("(%.*s)", prop_special->length, prop_special->data);
			break;
		}
		case FBX_PROPERTY_TYPE_ARRAY_FLOAT:
		{
			ASSERT((*it)->heap_data);
			const struct fbx_property_array *prop_array = (const struct fbx_property_array*)(*it)->heap_data;
			const float* prop_array_data = (const float*)prop_array->data;
			printf("{");
			for(uint32_t i = 0; i < prop_array->array_length; i++) {
				printf("%.2f ", prop_array_data[i]);
			}
			printf("}");
			break;
		}
		case FBX_PROPERTY_TYPE_ARRAY_INT32:
		{
			ASSERT((*it)->heap_data);
			const struct fbx_property_array *prop_array = (const struct fbx_property_array*)(*it)->heap_data;
			const int32_t* prop_array_data = (const int32_t*)prop_array->data;
			printf("{");
			for(uint32_t i = 0; i < prop_array->array_length; i++) {
				printf("%" PRId32 " ", prop_array_data[i]);
			}
			printf("}");
			break;
		}
		case FBX_PROPERTY_TYPE_ARRAY_DOUBLE:
		{
			ASSERT((*it)->heap_data);
			const struct fbx_property_array *prop_array = (const struct fbx_property_array*)(*it)->heap_data;
			const double* prop_array_data = (const double*)prop_array->data;
			printf("{");
			for(uint32_t i = 0; i < prop_array->array_length; i++) {
				printf("%.2f ", prop_array_data[i]);
			}
			printf("}");
			break;
		}
		case FBX_PROPERTY_TYPE_ARRAY_INT64:
			ASSERT_NOT_IMPLEMENTED();
			break;
		case FBX_PROPERTY_TYPE_ARRAY_BOOL:
			ASSERT_NOT_IMPLEMENTED();
			break;
		case FBX_PROPERTY_TYPE_ARRAY_CHAR:
			ASSERT_NOT_IMPLEMENTED();
			break;
		}

		printf("\n");

		//printf("%*sspecial_prop: %.*s\n", debug_indent * 4, "", special->length, special->data);
	}
}

static void fbx_print_node(const struct fbx_node *node, int indent)
{
	printf("%*s`%.*s` (%d children, %d props)\n",
		indent * 4, "",
		node->name_len, node->name,
		node->children ? alist_count(node->children) : 0,
		node->properties_count);

	indent++;

	if(node->properties) {
		fbx_print_property_list(node->properties, indent);
	}

	if(node->children) {
		fbx_print_node_list(node->children, indent);
	}
}

static void fbx_property_free(struct fbx_property *prop)
{
	free(prop->heap_data);
	prop->heap_data = NULL;
	free(prop);
}

static struct fbx_property_array* fbx_property_array_new(size_t element_size, struct blob_cur *cur)
{
	uint32_t array_length = 0;
	uint32_t encoding = 0;
	uint32_t compressed_length = 0;
	if(!blob_cur_read(array_length, cur)
		|| !blob_cur_read(encoding, cur)
		|| !blob_cur_read(compressed_length, cur)) {
		ASSERT_FAIL("FBX: Failed to read property array header");
		return NULL;
	}

	size_t array_size = array_length * element_size;
	struct fbx_property_array* prop_array = calloc(1, sizeof(struct fbx_property_array) + array_size);
	prop_array->array_length = array_length;
	prop_array->encoding = encoding;
	prop_array->compressed_length = compressed_length;

	if(prop_array->encoding) {
		if(blob_cur_can_read(cur, compressed_length)
			&& stbi_zlib_decode_buffer(prop_array->data, array_size, cur->it, prop_array->compressed_length) != array_size) {
			ASSERT_FAIL("FBX: Property array decode failed");
			goto fail;
		}
		if(!blob_cur_advance(cur, prop_array->compressed_length)) {
			ASSERT_FAIL("FBX: Failed to advance data cursor");
			goto fail;
		}
	} else {
		if(!blob_cur_read_type(prop_array->data, array_size, cur)) {
			ASSERT_FAIL("FBX: Failed to read property array data");
			goto fail;
		}
	}

	return prop_array;

fail:
	free(prop_array);
	return NULL;
}

static struct fbx_property* fbx_property_new(struct blob_cur *cur)
{
	struct fbx_property* prop = (struct fbx_property*) calloc(1, sizeof(struct fbx_property));
	if(!prop) {
		ASSERT_FAIL("FBX: Failed to allocate property");
		return NULL;
	}
	prop->heap_data = NULL;

	if(!blob_cur_read(prop->type, cur)) {
		ASSERT_FAIL("FBX: Failed to read property type");
		goto fail;
	}

	switch(prop->type) {
	case FBX_PROPERTY_TYPE_INT16:
	{
		if(!blob_cur_read_type(prop->static_data, sizeof(int16_t), cur)) {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_BOOL:
	{
		if(!blob_cur_read_type(prop->static_data, sizeof(char), cur)) {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_INT32:
	{
		if(!blob_cur_read_type(prop->static_data, sizeof(int32_t), cur)) {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_FLOAT:
	{
		if(!blob_cur_read_type(prop->static_data, sizeof(float), cur)) {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_DOUBLE:
	{
		if(!blob_cur_read_type(prop->static_data, sizeof(double), cur)) {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_INT64:
	{
		if(!blob_cur_read_type(prop->static_data, sizeof(int64_t), cur)) {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_BINARY:
	case FBX_PROPERTY_TYPE_STRING:
	{
		uint32_t special_size = 0;
		if(!blob_cur_read(special_size, cur)) {
			ASSERT_FAIL("FBX: Failed to read special property size");
			goto fail;
		}

		struct fbx_property_special* special = calloc(1, sizeof(struct fbx_property_special) + special_size);
		special->length = special_size;
		prop->heap_data = special;

		if(!blob_cur_read_type(special->data, special_size, cur)) {
			ASSERT_FAIL("FBX: Property data underrun");
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_ARRAY_FLOAT:
	{
		struct fbx_property_array *array = fbx_property_array_new(sizeof(float), cur);
		if(array) {
			prop->heap_data = array;
		} else {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_ARRAY_INT32:
	{
		struct fbx_property_array *array = fbx_property_array_new(sizeof(int32_t), cur);
		if(array) {
			prop->heap_data = array;
		} else {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_ARRAY_DOUBLE:
	{
		struct fbx_property_array *array = fbx_property_array_new(sizeof(double), cur);
		if(array) {
			prop->heap_data = array;
		} else {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_ARRAY_INT64:
	{
		struct fbx_property_array *array = fbx_property_array_new(sizeof(int64_t), cur);
		if(array) {
			prop->heap_data = array;
		} else {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_ARRAY_BOOL:
	{
		struct fbx_property_array *array = fbx_property_array_new(sizeof(char), cur);
		if(array) {
			prop->heap_data = array;
		} else {
			goto fail;
		}
		break;
	}
	case FBX_PROPERTY_TYPE_ARRAY_CHAR:
	{
		struct fbx_property_array *array = fbx_property_array_new(sizeof(char), cur);
		if(array) {
			prop->heap_data = array;
		} else {
			goto fail;
		}
		break;
	}
	default:
		ASSERT_FAIL("FBX: Unknown property type");
		goto fail;
	}

	return prop;

fail:
	fbx_property_free(prop);
	return NULL;
}

static void fbx_node_free(struct fbx_node *node)
{
	if(node->properties) {
		alist_free(node->properties, 1);
		node->properties = NULL;
	}
	if(node->children) {
		foreach_alist(struct fbx_node*, it, index, node->children) {
			fbx_node_free(it);
		}
		alist_free(node->children, 0);
		node->children = NULL;
	}
	free(node);
}

static struct alist* fbx_node_read_children(struct blob_cur *cur);

static struct fbx_node* fbx_node_new(size_t end_offset, struct blob_cur *cur)
{
	ASSERT(end_offset);

	struct fbx_node *node = (struct fbx_node*)calloc(1, sizeof(struct fbx_node));
	node->end_offset = end_offset;

	if(!blob_cur_read(node->properties_count, cur)
		|| !blob_cur_read(node->properties_list_len, cur)
		|| !blob_cur_read(node->name_len, cur))
	{
		ASSERT_FAIL("FBX: Failed to read node header");
		goto fail;
	}

	ASSERT(node->name_len);

	if(!(blob_cur_read_type(node->name, node->name_len, cur))) {
		ASSERT_FAIL("FBX: Failed to read node name");
		goto fail;
	}

	if(node->properties_count > 0) {
		node->properties = alist_new(node->properties_count);

		for(uint32_t i = 0; i < node->properties_count; i++) {
			struct fbx_property *prop = fbx_property_new(cur);
			if(!prop) {
				ASSERT_FAIL("FBX: Failed to read property");
				goto fail;
			}
			alist_append(node->properties, prop);
		}
	}

	if(!blob_cur_is_empty(cur))
	{
		node->children = fbx_node_read_children(cur);
		if(!node->children) {
			ASSERT_FAIL("FBX: Failed to read children");
			goto fail;
		}
	}

	return node;

fail:
	fbx_node_free(node);
	return NULL;
}

static struct alist* fbx_node_read_children(struct blob_cur *cur)
{
	ASSERT(!blob_cur_is_empty(cur));

	struct alist *children = alist_new(8);

	while(!blob_cur_is_empty(cur)) {
		while(blob_cur_remaining(cur) > FBX_NULL_NODE_SIZE) {
			size_t end_offset = 0;
			if(!blob_cur_read(end_offset, cur)) {
				ASSERT_FAIL("FBX: Failed to read node header");
				goto fail;
			}

			if(end_offset == 0) {
				return children;
			}

			struct blob_cur child_cur = blob_cur_make_from_start(cur, end_offset);
			struct fbx_node *child = fbx_node_new(end_offset, &child_cur);
			if(!child) {
				ASSERT_FAIL("FBX: Failed to parse root child");
				goto fail;
			}
			alist_append(children, child);
			if(!blob_cur_mirror(cur, &child_cur)) {
				ASSERT_FAIL("FBX: Failed to mirror data cursor");
				goto fail;
			}
		}

		char null_node_candidate[FBX_NULL_NODE_SIZE] = { 0xff };
		static char null_node[FBX_NULL_NODE_SIZE] = { 0x00 };
		if((blob_cur_read(null_node_candidate, cur)
			&& memcmp(null_node_candidate, null_node, FBX_NULL_NODE_SIZE) == 0)) {
			//ASSERT_FAIL("Found null record");
			break;
		} else {
			ASSERT_FAIL("FBX: Failed to read null node");
			goto fail;
		}
	}

	return children;

fail:
	if(children) {
		foreach_alist(struct fbx_node*, it, index, children) {
			fbx_node_free(it);
		}
		alist_free(children, 0);
	}
	return NULL;
}

struct fbx* fbx_new(const char *buf, size_t buf_size)
{
	struct fbx *fbx = (struct fbx*) calloc(1, sizeof(struct fbx));

	struct blob_cur cursor = blob_cur_make(buf, buf_size);
	struct blob_cur *cur = &cursor;

	if(!blob_cur_read(fbx->magic, cur)
		|| !blob_cur_read(fbx->pad, cur)
		|| !blob_cur_read(fbx->version, cur)) {
		ASSERT_FAIL("FBX: Failed to read header");
		goto fail;
	}

	if(!strview_equals(strview_static(fbx->magic), strview_static(FBX_FILE_MAGIC))) {
		ASSERT_FAIL("FBX: Incorrect header magic");
		goto fail;
	}

	if(!(fbx->pad[0] == 0x1a && fbx->pad[1] == 0x00)) {
		ASSERT_FAIL("FBX: Incorrect header padding");
		//goto fail;
	}

	fbx->children = fbx_node_read_children(cur);
	if(!fbx->children) {
		ASSERT_FAIL("FBX: Failed to read root children");
		goto fail;
	}

	fbx_print(fbx);
	
	printf("FBX: bytes remaining after parse: %u\n", (unsigned int)blob_cur_remaining(cur));
	
	//ASSERT_MESSAGE(blob_cur_is_empty(cur), "FBX: Did not consume all bytes");
	// TODO(TS): fail if cur != end ??

	return fbx;

fail:
	fbx_free(fbx);
	return NULL;
}

void fbx_free(struct fbx *fbx)
{
	foreach_alist(struct fbx_node*, it, index, fbx->children) {
		fbx_node_free(it);
	}
	alist_free(fbx->children, 0);
	free(fbx);
}

void fbx_print(const struct fbx *fbx)
{
	fbx_print_header(fbx);
	fbx_print_node_list(fbx->children, 0);
}
