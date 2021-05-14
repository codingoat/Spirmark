#pragma once

enum ShaderProgramType { glsl = 0, glslOpt = 1, spirv = 2, spirvOpt = 3, spirvMultiOpt = 4, glslOptSpirvOpt = 5, spirvOptGlsl = 6, shaderProgramTypeCount = 7 };
constexpr const char* shaderProgramTypeNames[shaderProgramTypeCount] = { "glsl", "glslOpt", "spirv", "spirvOpt", "spirvMultiOpt", "glslOptSpirvOpt", "spirvOptGlsl" };
constexpr const char* shaderProgramTypeDescriptions[shaderProgramTypeCount] = {
	"No optimization applied",
	"Applied glsl-optimizer",
	"Compiled to SPIR-V",
	"Compiled to SPIR-V and applied SPIRV-Tools Optimizer",
	"Compiled to SPIR-V and applied SPIRV-Tools Optimizer multiple times\n"
		"(until the code does not change, or the user set limit is reached),\n"
		"translating to GLSL and back to SPIR-V between each iteration",
	"Applied glsl-optimizer, compiled to SPIR-V and SPIRV-Tools Optimizer",
	"Compiled to SPIR-V, applied SPIRV-Tools Optimizer, and translated back to GLSL"
}; // used for UI tooltips