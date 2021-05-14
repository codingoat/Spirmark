#pragma once

// Helpers
#include <ProgramObject.h>
#include <Bencher.h>
#include <ShaderToySource.h>
#include <SpirverAstAnalyzer.h>
#include <BenchmarkedShaderResult.h>

// Spirmark components
#include <SpirmarkCore.h>
#include <SpirmarkUI.h>

#include <imgui/imgui.h>
#include <filesystem>

namespace Spirmark
{

class SpirmarkApp
{
public:
	SpirmarkApp();
	~SpirmarkApp();

	// main loop
	void Update();
	void Render();

	// input events
	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);

private:
	SpirmarkCore core;
	SpirmarkUI ui;
};

}