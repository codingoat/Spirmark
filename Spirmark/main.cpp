// GLEW, SDL, ImGui
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

// standard
#include <iostream>
#include <sstream>
#include "resource.h"

#include <SpirmarkApp.h>

SDL_Window* win = 0;

int main( int argc, char* args[] )
{
	// initialization

	if ( SDL_Init( SDL_INIT_VIDEO ) == -1 )
	{
		std::cout << "[SDL ind�t�sa]Hiba az SDL inicializ�l�sa k�zben: " << SDL_GetError() << std::endl;
		return 1;
	}
			
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,         32);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,		1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,          24);

	// antialiasing
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

    win = SDL_CreateWindow( "Spirmark",
							100, // pos x
							100, // pos y
							1920, // res x
							1080, // res y
							SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (win == 0)
	{
		std::cout << "SDL initialization error: " << SDL_GetError() << std::endl;
        return 1;
    }

	SDL_GLContext context = SDL_GL_CreateContext(win);
    if (context == 0)
	{
		std::cout << "SDL/OGL context error: " << SDL_GetError() << std::endl;
        return 1;
    }	

	SDL_GL_SetSwapInterval(0); // no vsync

	GLenum error = glewInit();
	if ( error != GLEW_OK )
	{
		std::cout << "GLEW error." << std::endl;
		return 1;
	}

	int glVersion[2] = {-1, -1}; 
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); 
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); 
	std::cout << "Running OpenGL " << glVersion[0] << "." << glVersion[1] << std::endl;

	if ( glVersion[0] == -1 && glVersion[1] == -1 )
	{
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow( win );
		std::cout << "OpenGL initalization error." << std::endl;
		return 1;
	}

	SDL_Surface* icon = IMG_Load("Assets/logo_32.png");
	SDL_SetWindowIcon(win, icon);
	std::stringstream window_title;
	window_title << "Spirmark (OpenGL " << glVersion[0] << "." << glVersion[1] << ")";
	SDL_SetWindowTitle(win, window_title.str().c_str());

	// Imgui init
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(win, context);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool quit = false;
	SDL_Event ev;

	Spirmark::SpirmarkApp app;

	int w, h;
	SDL_GetWindowSize(win, &w, &h);
	app.Resize(w, h);

	// main loop
	while (!quit)
	{
		// input loop
		while (SDL_PollEvent(&ev))
		{
			ImGui_ImplSDL2_ProcessEvent(&ev);
			bool is_mouse_captured = ImGui::GetIO().WantCaptureMouse;
			bool is_keyboard_captured = ImGui::GetIO().WantCaptureKeyboard;
			switch (ev.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (ev.key.keysym.sym == SDLK_ESCAPE)
					quit = true;
				if (!is_keyboard_captured)
					app.KeyboardDown(ev.key);
				break;
			case SDL_KEYUP:
				if (!is_keyboard_captured)
					app.KeyboardUp(ev.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (!is_mouse_captured)
					app.MouseDown(ev.button);
				break;
			case SDL_MOUSEBUTTONUP:
				if (!is_mouse_captured)
					app.MouseUp(ev.button);
				break;
			case SDL_MOUSEWHEEL:
				if (!is_mouse_captured)
					app.MouseWheel(ev.wheel);
				break;
			case SDL_MOUSEMOTION:
				if (!is_mouse_captured)
					app.MouseMove(ev.motion);
				break;
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					app.Resize(ev.window.data1, ev.window.data2);
				}
				break;
			}
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(win);
		ImGui::NewFrame();

		app.Update();
		app.Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(win);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow( win );

	return 0;
}