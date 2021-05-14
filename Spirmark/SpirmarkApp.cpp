#include "SpirmarkApp.h"

#include <math.h>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <sstream>     // ui strings

#include <array>
#include <list>
#include <tuple>
#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include <glsl_optimizer.h>
#include <ios>
#include "Spirver.h"
#include <iomanip>
#include <OpenXLSX.hpp>
#include <variant>

using namespace Spirmark;

SpirmarkApp::SpirmarkApp() : ui(core) {}

SpirmarkApp::~SpirmarkApp()
{
	Spirver::Clean();
}

void SpirmarkApp::Update()
{
	core.Update();
	ui.Update();
}

void SpirmarkApp::Render()
{
	core.Render();
	ui.RenderUI();
}

void SpirmarkApp::KeyboardDown(SDL_KeyboardEvent& key)
{
}
void SpirmarkApp::KeyboardUp(SDL_KeyboardEvent& key)
{
}

void SpirmarkApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	core.SetMousePos(glm::vec2(mouse.x, mouse.y), false);
}

void SpirmarkApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
	core.SetMousePos(glm::vec2(mouse.x, mouse.y), true);
}

void SpirmarkApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void SpirmarkApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

void SpirmarkApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );
	core.SetResolution(glm::vec3(_w, _h, 1));
}
