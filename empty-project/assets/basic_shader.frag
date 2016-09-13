#version 400

in vec2 texcoord;
out vec4 frag_color;

uniform vec4 color;
uniform sampler2D tex;

void main() {
    frag_color = texture(tex, vec2(texcoord.x, texcoord.y)) * color;
}
