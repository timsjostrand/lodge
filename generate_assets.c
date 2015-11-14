#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vfs.h"
#include "alist.h"

struct alist* assets_list;
struct alist* assets_list_textures;
struct alist* assets_list_sounds;
struct alist* assets_list_shaders;
struct alist* assets_list_misc;

#define MAX_ASSETS 512

void write_clean_name(FILE *fp, char* name)
{
	if (48 <= *name && *name <= 57)
	{
		fprintf(fp, "_");
	}

	for (int i = 0, i_size = strlen(name); i < i_size; i++)
	{
		char c = name[i];

		if (c == '.')
		{
			break;
		}
		else if (48 <= c && c <= 57 || 65 <= c && c <= 90 || 97 <= c && c <= 122 || c == 95)
		{
			fprintf(fp, "%c", c);
		}
		else if (c == ' ')
		{
			fprintf(fp, "_");
		}
		else if (c == '/')
		{
			fprintf(fp, "_");
		}
	}
}

void write_assets_c()
{
	FILE *fp;
	fp = fopen("assets.c", "w+");

	fprintf(fp, "#include \"assets.h\"\n\n");
	fprintf(fp, "#include \"vfs.h\"\n");
	fprintf(fp, "#include \"texture.h\"\n");
	fprintf(fp, "#include \"core_reload.h\"\n");
	fprintf(fp, "\n");

	fprintf(fp, "struct assets assets = { { 0 } };\n");
	fprintf(fp, "\n");

	fprintf(fp, "struct assets* assets_get()\n");
	fprintf(fp, "{\n");
	fprintf(fp, "\treturn &assets;\n");
	fprintf(fp, "}\n\n");

	fprintf(fp, "void assets_load()\n");
	fprintf(fp, "{\n");
	fprintf(fp, "\t// Textures\n");
	foreach_alist(char*, asset, i, assets_list_textures)
	{
		fprintf(fp, "\tvfs_register_callback(\"");
		fprintf(fp, "%s\", &core_reload_texture, &assets.textures.", asset);
		write_clean_name(fp, asset);
		fprintf(fp, ");\n");
	}
	fprintf(fp, "\n\t// Sounds\n");
	foreach_alist(char*, asset, i, assets_list_sounds)
	{
		fprintf(fp, "\tvfs_register_callback(\"");
		fprintf(fp, "%s\", &core_reload_sound, &assets.sounds.", asset);
		write_clean_name(fp, asset);
		fprintf(fp, ");\n");
	}
	fprintf(fp, "\n\t// Shaders\n");
	foreach_alist(char*, asset, i, assets_list_shaders)
	{
		fprintf(fp, "\tvfs_register_callback(\"");
		fprintf(fp, "%s\", &core_reload_shader, &assets.shaders.", asset);
		write_clean_name(fp, asset);
		fprintf(fp, ");\n");
	}
	fprintf(fp, "}\n\n");

	fprintf(fp, "void assets_release()\n");
	fprintf(fp, "{\n");
	fprintf(fp, "\t// Textures\n");
	foreach_alist(char*, asset, i, assets_list_textures)
	{
		fprintf(fp, "\ttexture_free(assets.textures.");
		write_clean_name(fp, asset);
		fprintf(fp, ");\n");
	}
	fprintf(fp, "\n\t// Sounds\n");
	foreach_alist(char*, asset, i, assets_list_sounds)
	{
		fprintf(fp, "\tsound_buf_free(assets.sounds.");
		write_clean_name(fp, asset);
		fprintf(fp, ");\n");
	}
	fprintf(fp, "\n\t// Shaders\n");
	foreach_alist(char*, asset, i, assets_list_shaders)
	{
		fprintf(fp, "\tshader_free(&assets.shaders.");
		write_clean_name(fp, asset);
		fprintf(fp, ");\n");
	}
	fprintf(fp, "}\n");
	fclose(fp);
}

void write_assets_h()
{
	FILE *fp;
	fp = fopen("assets.h", "w+");

	fprintf(fp, "#ifndef ASSETS_H\n");
	fprintf(fp, "#define ASSETS_H\n\n");
	fprintf(fp, "#include \"game.h\"\n");
	fprintf(fp, "#include \"shader.h\"\n");
	fprintf(fp, "#include \"sound.h\"\n\n");

	fprintf(fp, "struct shader;\n");

	fprintf(fp, "struct textures\n");
	fprintf(fp, "{\n");
	if(alist_count(assets_list_textures) > 0)
	{
		foreach_alist(char*, asset, i, assets_list_textures)
		{
			fprintf(fp, "\t");
			fprintf(fp, "GLuint\t");
			write_clean_name(fp, asset);
			fprintf(fp, ";");
			fprintf(fp, "\n");
		}
	}
	else
	{
		fprintf(fp, "\tint\t__none;\n");
	}
	fprintf(fp, "};\n\n");

	fprintf(fp, "struct sounds\n");
	fprintf(fp, "{\n");
	if(alist_count(assets_list_sounds) > 0)
	{
		foreach_alist(char*, asset, i, assets_list_sounds)
		{
			fprintf(fp, "\t");
			fprintf(fp, "sound_buf_t\t");
			write_clean_name(fp, asset);
			fprintf(fp, ";");
			fprintf(fp, "\n");
		}
	}
	else
	{
		fprintf(fp, "\tint\t__none;\n");
	}
	fprintf(fp, "};\n\n");

	fprintf(fp, "struct shaders\n");
	fprintf(fp, "{\n");
	if(alist_count(assets_list_shaders) > 0)
	{
		foreach_alist(char*, asset, i, assets_list_shaders)
		{
			if (strstr(asset, ".frag") == 0)
			{
				continue;
			}

			fprintf(fp, "\t");
			fprintf(fp, "struct shader ");
			write_clean_name(fp, asset);
			fprintf(fp, ";");
			fprintf(fp, "\n");
		}
	}
	else
	{
		fprintf(fp, "\tint\t__none;\n");
	}
	fprintf(fp, "};\n\n");

	fprintf(fp, "struct assets\n");
	fprintf(fp, "{\n");
	fprintf(fp, "\t struct textures textures;\n");
	fprintf(fp, "\t struct sounds sounds;\n");
	fprintf(fp, "\t struct shaders shaders;\n");
	fprintf(fp, "};\n\n");

	fprintf(fp, "struct assets assets;\n\n");

	fprintf(fp, "struct assets* assets_get();\n");
	fprintf(fp, "void assets_load();\n");
	fprintf(fp, "void assets_release();\n");

	fprintf(fp, "\n#endif //ASSETS_H\n");

	fclose(fp);
}

void add_assets()
{
	for (int i = 0, i_size = vfs_file_count(); i < i_size; i++)
	{
		alist_append(assets_list, vfs_get_simple_name(i));
	}

	const char* ext_texture[] = { ".png", ".tga", ".jpeg", ".jpg", ".bmp", ".psd", ".gif", ".hdr", ".pic", ".pnm" };
	const char* ext_sounds[] = { ".ogg" };
	const char* ext_shaders[] = { ".frag", ".vert" };

	foreach_alist(char*, asset, index, assets_list)
	{
		int added = 0;

		for (int i = 0, i_size = sizeof(ext_texture) / sizeof(ext_texture[0]); i < i_size; i++)
		{
			if (strstr(asset, ext_texture[i]) != 0)
			{
				alist_append(assets_list_textures, asset);
				added = 1;
				break;
			}
		}

		if (added)
		{
			continue;
		}

		for (int i = 0, i_size = sizeof(ext_sounds) / sizeof(ext_sounds[0]); i < i_size; i++)
		{
			if (strstr(asset, ext_sounds[i]) != 0)
			{
				alist_append(assets_list_sounds, asset);
				added = 1;
				break;
			}
		}

		if (added)
		{
			continue;
		}

		for (int i = 0, i_size = sizeof(ext_shaders) / sizeof(ext_shaders[0]); i < i_size; i++)
		{
			if (strstr(asset, ext_shaders[i]) != 0)
			{
				alist_append(assets_list_shaders, asset);
				added = 1;
				break;
			}
		}

		if (added)
		{
			continue;
		}

		alist_append(assets_list_misc, asset);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("VFS mount directory not set for generate_assets");
		return 0;
	}

	assets_list = alist_new(MAX_ASSETS);
	assets_list_textures = alist_new(MAX_ASSETS);
	assets_list_sounds = alist_new(MAX_ASSETS);
	assets_list_shaders = alist_new(MAX_ASSETS);
	assets_list_misc = alist_new(MAX_ASSETS);

	vfs_init(argv[1]);
	add_assets(assets_list);

	write_assets_c();
	write_assets_h();

	return 0;
}
