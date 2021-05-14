#pragma once

#include <imgui/imgui.h>
#include <SpirmarkCore.h>
#include <imgui/imgui_plot.h>

namespace Spirmark
{

class SpirmarkUI
{
public:
	SpirmarkUI(SpirmarkCore& core);

	// lifecycle
	void Update();
	void RenderUI();

private:
	SpirmarkCore& core;

	// windows
	void RenderPopups();
	void RenderShaderLoaderWindow();
	void RenderBenchmarkWindow();
	void RenderSettingsWindow();
	void RenderStatsWindow();
	void RenderPerfWindow();

	// colors, fonts
	enum UIColorId { background = 0, backgroundDark = 1, backgroundTranslucent = 2, interactable = 3, interactableLight = 4, interactableLighter = 5, accent = 6, accentDark = 7, accentLight = 8, UIColorIdCount = 9 };
	// https://colorhunt.co/palette/2763
	ImVec4 uiColors[UIColorIdCount] = {
		/*background*/ ImVec4(0.07f, 0.09f, 0.11f, 1.00f),
		/*backgroundDark*/ ImVec4(0.06f, 0.07f, 0.09f, 1.00f),
		/*backgroundTranslucent*/ ImVec4(0.07f, 0.09f, 0.11f, 0.78f),
		/*interactable*/ ImVec4(0.13f, 0.16f, 0.19f, 1.00f),
		/*interactableLight*/ ImVec4(0.22f, 0.24f, 0.27f, 1.00f),
		/*interactableLightLight*/ ImVec4(0.29f, 0.31f, 0.35f, 1.00f),
		/*accent*/ ImVec4(0.00f, 0.68f, 0.71f, 1.00f),
		/*accentDark*/ ImVec4(0.00f, 0.54f, 0.56f, 1.00f),
		/*accentLight*/ ImVec4(1.00f, 1.00f, 1.00f, 1.00f)
	};
	ImFont* fontSans = nullptr, * fontMono = nullptr, * fontTitle = nullptr, * fontHeader = nullptr;
	ImGui::PlotConfig frametimePlotConfig;

	// errors
	std::string errorMsgDisplayed;
	std::filesystem::path* pathToBeEdited = nullptr;

	// performance window
	float perfDisplayTimer = 0; // how much time left until next refresh?
	FrameTime perfDisplayFrameTime;
	int perfDisplayFps;
};

}



