#include "fbx_asset.h"

#include <stdint.h>
#include <GL/glew.h>

#include "math4.h"
#include "lodge_platform.h"
#include "graphics.h"
#include "fbx.h"
#include "str.h"

static struct fbx_property* fbx_asset_get_array_double(struct fbx *fbx, const char *path[], size_t path_count)
{
	struct fbx_node *node = fbx_get_node(fbx, path, path_count);
	if(node) {
		const uint32_t property_count = fbx_node_get_property_count(node);
		for(uint32_t i = 0; i < property_count; i++) {
			struct fbx_property *prop = fbx_node_get_property(node, i);
			if(fbx_property_get_type(prop) == FBX_PROPERTY_TYPE_ARRAY_DOUBLE) {
				return prop;
			}
		}
	}
	ASSERT_FAIL("fbx_asset: failed to find array property");
	return NULL;
}

static struct fbx_property* fbx_asset_get_array_int32(struct fbx *fbx, const char *path[], size_t path_count)
{
	struct fbx_node *node = fbx_get_node(fbx, path, path_count);
	if(node) {
		const uint32_t property_count = fbx_node_get_property_count(node);
		for(uint32_t i = 0; i < property_count; i++) {
			struct fbx_property *prop = fbx_node_get_property(node, i);
			if(fbx_property_get_type(prop) == FBX_PROPERTY_TYPE_ARRAY_INT32) {
				return prop;
			}
		}
	}
	ASSERT_FAIL("fbx_asset: failed to find array property");
	return NULL;
}

static struct fbx_string fbx_asset_get_string(struct fbx *fbx, const char *path[], size_t path_count)
{
	struct fbx_node *node = fbx_get_node(fbx, path, path_count);
	if(node) {
		const uint32_t property_count = fbx_node_get_property_count(node);
		for(uint32_t i = 0; i < property_count; i++) {
			struct fbx_property *prop = fbx_node_get_property(node, i);
			if(fbx_property_get_type(prop) == FBX_PROPERTY_TYPE_STRING) {
				return fbx_property_get_string(prop);
			}
		}
	}
	ASSERT_FAIL("fbx_asset: failed to find string property");
	static const struct fbx_string empty = { 0 };
	return empty;
}

struct fbx_asset fbx_asset_make(struct fbx *fbx)
{
	struct fbx_asset asset = { 0 };

	glGenVertexArrays(1, &asset.vertex_array_object);
	GL_OK_OR_GOTO(fail);

	glBindVertexArray(asset.vertex_array_object);
	GL_OK_OR_GOTO(fail);

	/* Vertices */
	{
		glGenBuffers(1, &asset.buffer_object_vertices);
		GL_OK_OR_GOTO(fail);

		glBindBuffer(GL_ARRAY_BUFFER, asset.buffer_object_vertices);
		GL_OK_OR_GOTO(fail);

		static const char* path_vertices[] = { "Objects", "Geometry", "Vertices" };

		struct fbx_property *vertices_prop = fbx_asset_get_array_double(fbx, path_vertices, LODGE_ARRAYSIZE(path_vertices));
		if(!vertices_prop) {
			goto fail;
		}

		const double* vertices_prop_data = fbx_property_get_array_double(vertices_prop);
		const uint32_t vertices_prop_data_count = fbx_property_get_array_count(vertices_prop);

		glBufferData(GL_ARRAY_BUFFER, vertices_prop_data_count * sizeof(double), vertices_prop_data, GL_STATIC_DRAW);
		GL_OK_OR_GOTO(fail);

		glEnableVertexAttribArray(0);
		GL_OK_OR_GOTO(fail);

		glVertexAttribPointer(0,
			3,
			GL_DOUBLE,
			GL_FALSE,
			3 * sizeof(double),
			NULL
		);
		GL_OK_OR_GOTO(fail);
	}

	/* Normals */
	{
		/* Check mapping type */
		{
			static const char* path_normals_mapping_type[] = { "Objects", "Geometry", "LayerElementNormal", "MappingInformationType" };

			struct fbx_string normals_mapping_type_str = fbx_asset_get_string(fbx, path_normals_mapping_type, LODGE_ARRAYSIZE(path_normals_mapping_type));
			strview_t normals_mapping_type = strview_make(normals_mapping_type_str.data, normals_mapping_type_str.length);

			if( !strview_equals(normals_mapping_type, strview_static("ByPolygonVertex") ) ) {
				ASSERT_FAIL("Normal mapping != `ByPolygonVertex` not implemented");
				goto fail;
			}
		}
		
		/* Upload normal buffer */
		{
			static const char* path_normals[] = { "Objects", "Geometry", "LayerElementNormal", "Normals" };

			glGenBuffers(1, &asset.buffer_object_normals);
			GL_OK_OR_GOTO(fail);

			glBindBuffer(GL_ARRAY_BUFFER, asset.buffer_object_normals);
			GL_OK_OR_GOTO(fail);

			struct fbx_property *normals_prop = fbx_asset_get_array_double(fbx, path_normals, LODGE_ARRAYSIZE(path_normals));
			if(!normals_prop) {
				goto fail;
			}

			const double* normals_prop_data = fbx_property_get_array_double(normals_prop);
			const uint32_t normals_prop_data_count = fbx_property_get_array_count(normals_prop);

			glBufferData(GL_ARRAY_BUFFER, normals_prop_data_count * sizeof(double), normals_prop_data, GL_STATIC_DRAW);
			GL_OK_OR_GOTO(fail);

			glEnableVertexAttribArray(1);
			GL_OK_OR_GOTO(fail);

			glVertexAttribPointer(1,
				3,
				GL_DOUBLE,
				GL_FALSE,
				3 * sizeof(double),
				NULL
			);
			GL_OK_OR_GOTO(fail);
		}
	}

	/* Indices */
	{
		glGenBuffers(1, &asset.buffer_object_indices);
		GL_OK_OR_GOTO(fail);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asset.buffer_object_indices);
		GL_OK_OR_GOTO(fail);

		static const char* path_indices[] = { "Objects", "Geometry", "PolygonVertexIndex" };

		struct fbx_property *indices_prop = fbx_asset_get_array_int32(fbx, path_indices, LODGE_ARRAYSIZE(path_indices));
		if(!indices_prop) {
			goto fail;
		}

		const int32_t* indices_prop_data = fbx_property_get_array_int32(indices_prop);
		const uint32_t indices_prop_data_count = fbx_property_get_array_count(indices_prop);

		if(indices_prop_data_count < 3) {
			ASSERT_FAIL("Very small index buffer");
			goto fail;
		}

		int triangle_format = indices_prop_data[2] < 0;
		if(!triangle_format) {
			ASSERT_FAIL("Non triangle vertex format is not supported");
			goto fail;
		}

		/* Convert signed FBX indices to unsigned indices */
		uint32_t* indices = malloc(indices_prop_data_count * sizeof(uint32_t));
		for(uint32_t i = 0; i < indices_prop_data_count; i++) {
			const int32_t index = indices_prop_data[i];
			indices[i] = index >= 0 ? (uint32_t)index : (uint32_t)(~index);
		}

		asset.indices_count = indices_prop_data_count;

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_prop_data_count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		GL_OK_OR_GOTO(fail);

		free(indices);
	}

	/* Reset bindings to default */
	glBindVertexArray(0);
	GL_OK_OR_GOTO(fail);

	return asset;

fail:
	fbx_asset_reset(&asset);
	return asset;
}

void fbx_asset_reset(struct fbx_asset *asset)
{
	if(asset->buffer_object_indices) {
		glDeleteBuffers(1, &asset->buffer_object_indices);
	}
	if(asset->buffer_object_vertices) {
		glDeleteBuffers(1, &asset->buffer_object_vertices);
	}
	if(asset->buffer_object_normals) {
		glDeleteBuffers(1, &asset->buffer_object_normals);
	}
	if(asset->vertex_array_object) {
		glDeleteVertexArrays(1, &asset->vertex_array_object);
	}

	struct fbx_asset tmp = { 0 };
	*asset = tmp;
}

void fbx_asset_render(struct fbx_asset *asset, struct shader *shader, struct mvp mvp)
{
	glUseProgram(shader->program);
	GL_OK_OR_ASSERT("Failed to use shader program");

	{
		GLint uniform_projection = glGetUniformLocation(shader->program, "projection");
		glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, (const GLfloat*)mvp.projection.m);
		GL_OK_OR_ASSERT("Failed to set projection");
	}

	{
		GLint uniform_view = glGetUniformLocation(shader->program, "view");
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, (const GLfloat*)mvp.view.m);
		GL_OK_OR_ASSERT("Failed to set view");
	}
	
	{
		GLint uniform_model = glGetUniformLocation(shader->program, "model");
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, (const GLfloat*)mvp.model.m);
		GL_OK_OR_ASSERT("Failed to set model");
	}

	glBindVertexArray(asset->vertex_array_object);
	glDrawElements(GL_TRIANGLES, asset->indices_count, GL_UNSIGNED_INT, NULL);
	GL_OK_OR_ASSERT("Failed to draw");
}