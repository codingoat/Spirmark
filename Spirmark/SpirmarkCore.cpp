#include <SpirmarkCore.h>

#include <ObjParser_OGL3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <numeric>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <imgui/stb_image_write.h>
#include <Errors.h>
#include <iostream>

using namespace Spirmark;

#pragma region Public

SpirmarkCore::SpirmarkCore()
{
	similarityDisplayProgram.Init({
		{ GL_VERTEX_SHADER, "Assets/fullscreen.vert" },
		{ GL_FRAGMENT_SHADER, "Assets/similarity.frag" }
	});
	
	// load default shader and config
	IO::LoadApplicationSettings(spirvMultiOptIterCountMax, vsync[0]);
	IO::LoadBenchmarkConfig("Shaders/default.spirmark", false, benchmarkedShaderResults, benchmarkLength);
	loadedShaders.push_back(BenchmarkedShader("LiquidMetal.frag", std::filesystem::path("Assets/LiquidMetal.frag")));
	CompileLoadedShaderToys();

	suzanne = ObjParser::parse("Assets/suzanne.obj");
	suzanne->initBuffers();

	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);

	// init similarity
	stbi_flip_vertically_on_write(true);
	glGenFramebuffers(2, fbos);
	glGenTextures(2, textures); 
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // otherwise crash on glGetImage() delete[] much later
	glGenBuffers(1, &similarityAtomicBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, similarityAtomicBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 6, NULL, GL_DYNAMIC_DRAW);
}

SpirmarkCore::~SpirmarkCore()
{
	glDeleteTextures(2, textures);
	glDeleteFramebuffers(2, fbos);
	Spirver::Clean();
	IO::SaveApplicationSettings(spirvMultiOptIterCountMax, vsync[0]);
}

void SpirmarkCore::Update()
{
	// change vsync if needed
	if (vsync[0] != vsync[1])
	{
		vsync[1] = vsync[0];
		SDL_GL_SetSwapInterval(vsync[0] ? 1 : 0);
	}

	// set uniform values
	uniformValues.time = bencher.GetTimeSinceLastBenchmark();
	uniformValues.timeDelta = bencher.GetLastFrameTime().total / 1000;
	uniformValues.frame = bencher.GetTotalFrameCount();
}

void SpirmarkCore::Render()
{
	bencher.GpuStart();

	glClear(GL_COLOR_BUFFER_BIT);

	SetUniformsForRelevantShaders();

	// actual rendering
	if (!benchmarkStage || benchmarkMode == BenchmarkMode::SingleFullscreen)
	{
		loadedShaders[0].GetShaderPrograms()[activeShaderProgram].Use();
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	else if (benchmarkStage && benchmarkMode == BenchmarkMode::Similarity)
	{
		DoSimilarityBenchmarkForFrame();
	}
	else // models
	{
		int countX = std::max(1, int(std::round(4 / 3.0 * std::sqrt(suzanneCount))));
		int countY = suzanneCount / countX + (suzanneCount % countX ? 1 : 0);
		float margin[2] = { 16 / float(countX + 1) , 10 / float(countY + 1) };
		// screen safe zone: ~[(-6, -3), (6, 3)], using extended zone 

		for (int i = 0; i < suzanneCount; i++)
		{
			loadedShaders[i % loadedShaders.size()].GetShaderPrograms()[activeShaderProgram].Use();
			glm::mat4 MVP = Projection * View * glm::translate(glm::vec3(-8 + margin[0] * (i % countX + 1), 5 - margin[1] * (i / countX + 1), 0));
			glUniformMatrix4fv(99, 1, false, &MVP[0][0]);
			suzanne->draw();
		}
	}

	bencher.GpuDone();
	bencher.FrameDone();

	HandleBenchmarkState();
}

void SpirmarkCore::SetMousePos(const glm::vec2& pos, bool clicked)
{
	if (clicked) uniformValues.mouse = glm::vec4(pos.x, pos.y, pos.x, pos.y);
	else uniformValues.mouse = glm::vec4(pos.x, pos.y, uniformValues.mouse.z, uniformValues.mouse.w);
}

void SpirmarkCore::SetResolution(const glm::vec2& res)
{
	uniformValues.resolution = glm::vec3(res, 1);
}

#pragma endregion

void SpirmarkCore::DoSimilarityBenchmarkForFrame()
{
	// draw original
	glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
	loadedShaders[0].GetShaderPrograms()[glsl].Use();
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// draw optimized
	glBindFramebuffer(GL_FRAMEBUFFER, fbos[1]);
	loadedShaders[0].GetShaderPrograms()[activeShaderProgram].Use();
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// bind ssbo
	GLuint absDiff;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, similarityAtomicBuffer);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &absDiff);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, similarityAtomicBuffer);

	// draw visual difference
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	similarityDisplayProgram.Use();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	// get ssbo
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &absDiff);

	// get raw pixel data, calculate ssim
	GLubyte* pixelsO = new GLubyte[(long long unsigned)(uniformValues.resolution.x * uniformValues.resolution.y) * 3];
	GLubyte* pixelsM = new GLubyte[(long long unsigned)(uniformValues.resolution.x * uniformValues.resolution.y) * 3];
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsO);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsM);
	double ssim = CalculateStructuralSimilarity(pixelsO, pixelsM, uniformValues.resolution.x, uniformValues.resolution.y);
	delete[] pixelsO;
	delete[] pixelsM;

	// store calculated stats
	shaderSimilarityStats[benchmarkedShader - 1][benchmarkStage - 1].AddStat(ImageSimilarity(absDiff, ssim), uniformValues);
}

void SpirmarkCore::HandleBenchmarkState()
{
	if (bencher.IsBenchmarkDone() && benchmarkStage) // if the current benchmark stage is over
	{
		benchmarkedShaderResults[benchmarkedShader - 1].results[benchmarkStage - 1] = bencher.GetBenchmarkResults();
		benchmarkStage = (benchmarkStage + 1) % (shaderProgramTypeCount + 1);
		
		if(benchmarkStage) // we have stages left
		{
			// start next benchmark stage
			activeShaderProgram = benchmarkStage - 1;
			float benchlength = benchmarkLength * 60 / shaderProgramTypeCount;
			if (benchmarkMode != BenchmarkMode::MultiModel) benchlength /= benchmarkedShaderResults.size();
			bencher.StartBenchmark(benchlength);
		}
		else // done with stages
		{
			// export results relevant to current shader and benchmark
			if (exportBenchmarkedShaders) IO::ExportShaders(loadedShaders, false);
			if (benchmarkMode == Similarity) ExportWorstSimilarityImagesForCurrentShader(); 
			if (benchmarkMode != MultiModel) //get useful multi opt iters
			{
				benchmarkedShaderResults[benchmarkedShader - 1].usefulSpirvMultiOptIters = loadedShaders[0].GetSpirvMultiOptIterUseful();
			}
			else
			{
				for (int i = 0; i < loadedShaders.size(); i++)
					benchmarkedShaderResults[i].usefulSpirvMultiOptIters = loadedShaders[i].GetSpirvMultiOptIterUseful();
			}

			if (benchmarkMode != MultiModel && (benchmarkedShader = (benchmarkedShader + 1) % (benchmarkedShaderResults.size() + 1))) // we have shaders left
			{
				// start benchmark for next shader
				InitBenchmarkForShader();
				bencher.StartBenchmark(benchmarkLength * 60 / shaderProgramTypeCount / benchmarkedShaderResults.size());
				benchmarkStage++;
				activeShaderProgram = benchmarkStage - 1;
			}
			else // benchmark done
			{
				// export results
				if (benchmarkMode != Similarity) IO::ShaderResultsToFile((BenchmarkMode)benchmarkMode, benchmarkedShaderResults, uniformValues, benchmarkLength, spirvMultiOptIterCountMax, suzanneCount);
				else IO::ImageSimilaritiesToFile(shaderSimilarityStats, benchmarkedShaderResults, benchmarkLength);
				
				// update settings
				vsync[0] = vsync[2]; // Update() will set vsync if it needs changing
				SDL_SetWindowResizable(win, SDL_TRUE);

				// load a shader for non-benchmark mode
				loadedShaders.clear();
				loadedShaders.emplace_back(benchmarkedShaderResults[benchmarkedShaderResults.size() - 1].name, std::filesystem::path(benchmarkedShaderResults[benchmarkedShaderResults.size() - 1].path));
				CompileLoadedShaderToys();
			}
		}
	}
}

void SpirmarkCore::CompileLoadedShaderToys(bool model)
{
	for(auto& shader : loadedShaders)
    {
	    bool success = shader.Compile(model, spirvMultiOptIterCountMax);
	    if(!success)
        {
	        if(benchmarkStage) CancelBenchmark();
	        return;
        }
    }
}

void SpirmarkCore::ExportWorstSimilarityImagesForCurrentShader()
{
	// save location
	std::filesystem::path actualSaveLocation;
	IO::detail::TrySaveFolder(IO::saveLocations.simImg.string(), ".temp", actualSaveLocation);
	actualSaveLocation = actualSaveLocation.parent_path().string() + "/";

	// prepare buffers
	GLubyte* pixels = new GLubyte[(int)uniformValues.resolution.x * (int)uniformValues.resolution.y * 3];
	glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	for (int prog = 0; prog < shaderProgramTypeCount; prog++)
	{
		// draw original
		loadedShaders[0].GetShaderPrograms()[glsl].Use();
		shaderSimilarityStats[benchmarkedShader - 1][prog].worstUniformValues.Apply();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		std::string imgpath = actualSaveLocation.parent_path().string() + "/" + benchmarkedShaderResults[benchmarkedShader-1].name + "_" + shaderProgramTypeNames[prog] + "_target.png";
		stbi_write_png(imgpath.c_str(), uniformValues.resolution.x, uniformValues.resolution.y, 3, pixels, uniformValues.resolution.x * 3);

		// draw optimized
        loadedShaders[0].GetShaderPrograms()[prog].Use();
		shaderSimilarityStats[benchmarkedShader - 1][prog].worstUniformValues.Apply();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		imgpath = actualSaveLocation.parent_path().string() + "/" + benchmarkedShaderResults[benchmarkedShader-1].name + "_" + shaderProgramTypeNames[prog] + ".png";
		stbi_write_png(imgpath.c_str(), uniformValues.resolution.x, uniformValues.resolution.y, 3, pixels, uniformValues.resolution.x * 3);
	}

	delete[] pixels;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#pragma region Uniforms

void SpirmarkCore::SetUniformsForRelevantShaders()
{
	if (!benchmarkStage || benchmarkMode == SingleFullscreen || benchmarkMode == SingleModel)
	{
        loadedShaders[0].GetShaderPrograms()[activeShaderProgram].Use();
		uniformValues.Apply(!benchmarkStage);
	}
	else if (benchmarkMode == MultiModel)
	{
		for (int i = 0; i < loadedShaders.size(); i++)
		{
            loadedShaders[i].GetShaderPrograms()[activeShaderProgram].Use();
			uniformValues.Apply(!benchmarkStage);
		}
	}
	else if (benchmarkMode == Similarity)
	{
        loadedShaders[0].GetShaderPrograms()[glsl].Use();
		uniformValues.Apply(!benchmarkStage);
        loadedShaders[0].GetShaderPrograms()[activeShaderProgram].Use();
		uniformValues.Apply(!benchmarkStage);
	}
}

#pragma endregion

#pragma region Benchmark

void SpirmarkCore::StartBenchmark()
{
	// change application settings
	vsync[2] = vsync[0];
	vsync[0] = vsync[1] = false;
	SDL_GL_SetSwapInterval(0); // no vsync
	SDL_SetWindowResizable(win, SDL_FALSE); // not resizable

	// init ands start first stage
	benchmarkedShader = benchmarkStage = 1;
	activeShaderProgram = benchmarkStage - 1;
	InitBenchmarkForShader();
	benchmarkLength = std::max(benchmarkLength, benchmarkedShaderResults.size() * 0.5f); // don't allow too short benchmarks
	float benchlength = benchmarkLength * 60 / shaderProgramTypeCount;
	if (benchmarkMode != BenchmarkMode::MultiModel) benchlength /= benchmarkedShaderResults.size();
	bencher.StartBenchmark(benchlength);

	// similarity needs more FBO initialization
	if (benchmarkMode == BenchmarkMode::Similarity)
	{
		shaderSimilarityStats = std::vector<std::array<ImageSimilarityStats, shaderProgramTypeCount>>(benchmarkedShaderResults.size());
		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, uniformValues.resolution.x, uniformValues.resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
		}
	}
}

void SpirmarkCore::InitBenchmarkForShader()
{
	switch (benchmarkMode)
	{
	case Similarity:
	case SingleFullscreen:
	case SingleModel:
		loadedShaders[0] = BenchmarkedShader(benchmarkedShaderResults[benchmarkedShader - 1].name, std::filesystem::path(benchmarkedShaderResults[benchmarkedShader - 1].path));
		CompileLoadedShaderToys(benchmarkMode == SingleModel);
        std::copy(loadedShaders[0].GetShaderStats().begin(), loadedShaders[0].GetShaderStats().end(), benchmarkedShaderResults[benchmarkedShader - 1].stat);
		break;
	case MultiModel: // load all queued shaders
		loadedShaders.clear();
		loadedShaders.reserve(benchmarkedShaderResults.size());
		for (BenchmarkedShaderResult& sr : benchmarkedShaderResults)
		    loadedShaders.emplace_back(sr.name, std::filesystem::path(sr.path));
		CompileLoadedShaderToys(true);
		for (int i = 0; i < benchmarkedShaderResults.size(); i++) std::copy(loadedShaders[i].GetShaderStats().begin(), loadedShaders[i].GetShaderStats().end(), benchmarkedShaderResults[i].stat);
	}

	bencher.FrameDone(); // compilation shouldnt count into the start of the next benchmark, this prevents the long compilation delta from being added
}

void SpirmarkCore::CancelBenchmark()
{
	bencher.CancelBenchmark();
	// reset settings
	vsync[0] = vsync[2]; // Update will set the vsync if it needs changing
	SDL_SetWindowResizable(win, SDL_TRUE);
	benchmarkStage = benchmarkedShader = 0;
	if (benchmarkMode != SingleFullscreen) // load shader for non-benchmark mode
	{
		loadedShaders.clear();
		loadedShaders.emplace_back(benchmarkedShaderResults[0].name, std::filesystem::path(benchmarkedShaderResults[0].path));
		CompileLoadedShaderToys();
	}
}

#pragma endregion
