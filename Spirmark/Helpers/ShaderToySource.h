#pragma once

/// A simple class to transform a ShaderToy shader to a complete GLSL shader
class ShaderToySource
{
public:
	std::string name;
	std::string source;

	ShaderToySource(const char* name, const std::string&& stSource) : name(name)
	{
		source = "#version 460 core\nlayout(location = 0) out vec4 fs_out_col;\nlayout(location = 0) uniform vec3      iResolution;           // viewport resolution (in pixels)\nlayout(location = 1) uniform float     iTime;                 // shader playback time (in seconds)\nlayout(location = 2) uniform float     iTimeDelta;            // render time (in seconds)\nlayout(location = 3) uniform int       iFrame;                // shader playback frame\nlayout(location = 4) uniform float     iChannelTime[4];       // channel playback time (in seconds)\nlayout(location = 8) uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)\nlayout(location = 12) uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click\n"
			+ stSource
			+ "\nvoid main(){ mainImage(fs_out_col, gl_FragCoord.xy); }";
	}
};

