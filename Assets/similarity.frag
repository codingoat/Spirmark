#version 460 core

layout(location = 0) in vec2 vs_out_tex;
layout(location = 0) out vec4 fs_out_col;

layout(location = 0, binding = 0) uniform sampler2D original;
layout(location = 1, binding = 1) uniform sampler2D modified;

layout(binding = 0, offset = 0) uniform atomic_uint diff;

void main()
{
	vec3 colO = texture(original, vs_out_tex).xyz;
	vec3 colM = texture(modified, vs_out_tex).xyz;
	vec3 colDiff = abs(colO - colM);
	
	fs_out_col = vec4( colDiff * 50 + colO * 0.1, 1);
	
	int lumO = int((0.375 * colO.x + 0.5 * colO.y + 0.125 * colO.z) * 255);
	int lumM = int((0.375 * colM.x + 0.5 * colM.y + 0.125 * colM.z) * 255);
	
	atomicCounterAdd(diff, int((colDiff.x + colDiff.y + colDiff.z) * 255));
}