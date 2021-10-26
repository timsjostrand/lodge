//#version 450 core
//#extension GL_ARB_bindless_texture : require
//#extension GL_ARB_gpu_shader_int64 : require

uniform mat4 ProjMtx;

in vec2 Position;
in vec2 TexCoord;
in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
	Frag_UV = TexCoord;
	Frag_Color = Color;
	gl_Position = ProjMtx * vec4(Position.xy, 0, 1);
}