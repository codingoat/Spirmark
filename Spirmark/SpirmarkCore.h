#pragma once

// GLEW & SDL
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <Bencher.h>
#include <filesystem>
#include <ProgramObject.h>
#include <ShaderProgramType.h>
#include <ShaderToySource.h>
#include <BenchmarkedShaderResult.h>
#include <Mesh_OGL3.h>
#include <StructuralSimilarity.h>
#include <UniformValues.h>
#include <IO.h>
#include <Helpers/BenchmarkMode.h>
#include <Helpers/BenchmarkedShader.h>


extern SDL_Window* win;

namespace Spirmark 
{

class SpirmarkCore
{
	friend class SpirmarkUI;

public:
	SpirmarkCore();
	~SpirmarkCore();

	// lifecycle
	void Update();
	void Render();

	// input events
	void SetMousePos(const glm::vec2& mouse, bool clicked = false);
	void SetResolution(const glm::vec2& res);

private:
	// lifecycle
	void DoSimilarityBenchmarkForFrame();
	void HandleBenchmarkState();

	// Shaders
	void CompileLoadedShaderToys(bool model = false);
    std::vector<BenchmarkedShader> loadedShaders;
	int activeShaderProgram = glsl;
	int spirvMultiOptIterCountMax = 5;

	// Benchmark settings
    int benchmarkMode = BenchmarkMode::SingleFullscreen;
    Mesh* suzanne;
	int suzanneCount = 7;
	bool vsync[3] = { false }; //UI, lastframe, before benchmark

	// Similarity
	void ExportWorstSimilarityImagesForCurrentShader();
	GLuint fbos[2], textures[2]; // original, optimized
	ProgramObject similarityDisplayProgram;
	GLuint similarityAtomicBuffer; // stores the color difference sum calculated in the similarity fragment shader
	std::vector<std::array<ImageSimilarityStats, shaderProgramTypeCount>> shaderSimilarityStats;

	// Uniforms
	void SetUniformsForRelevantShaders();
	UniformValues uniformValues;
	glm::mat4 Projection = glm::perspective(45.0f, 1920 / (float)1080, 0.01f, 100.0f),
		View = glm::lookAt(glm::vec3(0, 0, 8), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Benchmark
	void StartBenchmark();
	void InitBenchmarkForShader();
	void CancelBenchmark();
	Bencher bencher;
	std::vector<BenchmarkedShaderResult> benchmarkedShaderResults;
	int benchmarkStage = 0; // which optimization variant is being benchmarked? indexed from 1, 0 means none
	int benchmarkedShader = 0; // which shader is being benchmarked? indexed from 1, 0 means none
	float benchmarkLength = 1; // in minutes
	bool exportBenchmarkedShaders = false;
};

}



