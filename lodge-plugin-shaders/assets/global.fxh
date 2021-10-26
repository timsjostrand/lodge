#version 430

// This is the global header, whatever goes inside here will be included in all
// other shader files (*.vert, *.frag, *.fxh), including any other files
// #included in this one.

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (2*M_PI)

uniform vec2 resolution;
