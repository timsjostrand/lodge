#version 400 core
precision highp float;

uniform mat4 transform;
uniform mat4 projection;
uniform float time;
uniform int sprite_type;
uniform float ball_last_hit_x;
uniform float ball_last_hit_y;

in vec3 vp;
in vec2 texcoord_in;
out vec2 texcoord;

void main() {
   texcoord = texcoord_in;
   gl_Position = projection * transform * vec4(vp, 1.0);
}
