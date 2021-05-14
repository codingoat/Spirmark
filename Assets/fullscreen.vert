#version 460 core

layout(location = 0) out vec2 vs_out_tex;

vec4 positions[3] = vec4[3](vec4(-1, -1, 0, 1), vec4(3, -1, 0, 1), vec4(-1, 3, 0, 1));

void main()
{
	gl_Position = positions[gl_VertexID];
	vs_out_tex = (positions[gl_VertexID].xy + 1) / 2;
}