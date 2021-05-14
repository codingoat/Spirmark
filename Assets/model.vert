#version 460

layout(location = 0) in vec3 vs_in_pos;
layout(location = 1) in vec3 vs_in_norm;
layout(location = 2) in vec2 vs_in_tex;

layout(location = 99) uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4( vs_in_pos, 1 );
}