#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace Spirmark
{
	enum UniformNames { iResolution = 0, iTime = 1, iTimeDelta = 2, iFrame = 3, iChannelTime = 4, iChannelResolution = 5, iMouse = 6, uniformNamesCount = 7 };
	static constexpr char* uniformNames[uniformNamesCount] = { "iResolution", "iTime", "iTimeDelta", "iFrame", "iChannelTime", "iChannelResolution", "iMouse" };
	static constexpr int uniformLocations[uniformNamesCount] = { 0, 1, 2, 3, 4, 8, 12 };
	
	static constexpr GLfloat mouseDefaultValues[4] = { 0, 0, 0, 0 };

	/// Stores the values of uniform variables for one frame
	struct UniformValues
	{
		// TODO: for multichannel support
		// sampler2D iChannel{i} : Sampler for input textures i
		// vec4 iDate : year, month, day, time in .xyzw
		// float iSampleRate : the sound sample rate (typically 44100)
		// iChannelResolution/Time : Not needed since we only have single channel shaders

		glm::vec3 resolution = glm::vec3();
		float time = 0;
		float timeDelta = 0;
		int frame = 0;
		glm::vec4 mouse = glm::vec4();

		UniformValues() {};
		UniformValues(glm::vec3 resolution, float time, float timeDelta, int frame, glm::vec4 mouse) :
			resolution(resolution), time(time), timeDelta(timeDelta), frame(frame), mouse(mouse) {};

		void Apply(bool benchmarking = false)
		{
			glUniform3fv(uniformLocations[iResolution], 1, &resolution[0]);
			glUniform1fv(uniformLocations[iTime], 1, &time);
			glUniform1fv(uniformLocations[iTimeDelta], 1, &timeDelta);
			glUniform1iv(uniformLocations[iFrame], 1, &frame);
			if(benchmarking) glUniform4fv(uniformLocations[iMouse], 1, &mouse[0]);
			else glUniform4fv(uniformLocations[iMouse], 1, mouseDefaultValues);
		}
	};
}
