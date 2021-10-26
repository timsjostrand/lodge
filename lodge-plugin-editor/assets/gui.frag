layout(binding=0) uniform sampler2D Texture;

in vec2 Frag_UV;
in vec4 Frag_Color;

out vec4 Out_Color;

void main()
{
	//sampler2D smp = sampler2D(Texture);
	Out_Color = Frag_Color * texture(Texture, Frag_UV.xy) * 1.0;
}