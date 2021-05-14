#include "SpirmarkUI.h"
#include <imgui/ImFileDialog.h>
#include <imgui/IconsFontAwesome5.h>
#include <UIWidgets.h>
#include <iostream>
#include <IO.h>
#include <Errors.h>
#include <Helpers/BenchmarkMode.h>

using namespace Spirmark;

SpirmarkUI::SpirmarkUI(SpirmarkCore& core) : core(core)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 4);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(.5, .5));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 6));
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 12);

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = uiColors[background];
	colors[ImGuiCol_PopupBg] = uiColors[backgroundDark];
	colors[ImGuiCol_FrameBg] = uiColors[interactable];
	colors[ImGuiCol_FrameBgHovered] = uiColors[interactableLight];
	colors[ImGuiCol_FrameBgActive] = uiColors[accent];
	colors[ImGuiCol_TitleBg] = uiColors[background];
	colors[ImGuiCol_TitleBgActive] = uiColors[accentDark];
	colors[ImGuiCol_TitleBgCollapsed] = uiColors[backgroundTranslucent];
	colors[ImGuiCol_MenuBarBg] = uiColors[background];
	colors[ImGuiCol_ScrollbarBg] = uiColors[backgroundDark];
	colors[ImGuiCol_ScrollbarGrab] = uiColors[interactable];
	colors[ImGuiCol_ScrollbarGrabHovered] = uiColors[interactableLight];
	colors[ImGuiCol_ScrollbarGrabActive] = uiColors[accent];
	colors[ImGuiCol_CheckMark] = uiColors[accentDark];
	colors[ImGuiCol_SliderGrab] = uiColors[accentDark];
	colors[ImGuiCol_SliderGrabActive] = uiColors[accentLight];
	colors[ImGuiCol_Button] = uiColors[interactableLight];
	colors[ImGuiCol_ButtonHovered] = uiColors[interactableLighter];
	colors[ImGuiCol_ButtonActive] = uiColors[accent];
	colors[ImGuiCol_Header] = uiColors[interactable];
	colors[ImGuiCol_HeaderHovered] = uiColors[interactableLight];
	colors[ImGuiCol_HeaderActive] = uiColors[accent];
	colors[ImGuiCol_Separator] = uiColors[interactable];
	colors[ImGuiCol_SeparatorHovered] = uiColors[interactableLight];
	colors[ImGuiCol_SeparatorActive] = uiColors[accent];
	colors[ImGuiCol_ResizeGrip] = uiColors[accentDark];
	colors[ImGuiCol_ResizeGripHovered] = uiColors[accent];
	colors[ImGuiCol_ResizeGripActive] = uiColors[accentLight];
	colors[ImGuiCol_Tab] = uiColors[interactableLight];
	colors[ImGuiCol_TabHovered] = uiColors[accent];
	colors[ImGuiCol_TabActive] = uiColors[accentDark];
	colors[ImGuiCol_TableHeaderBg] = uiColors[interactableLight];
	colors[ImGuiCol_TextSelectedBg] = uiColors[accentDark];
	colors[ImGuiCol_DragDropTarget] = uiColors[accent];
	colors[ImGuiCol_NavHighlight] = uiColors[accent];

	ImFontConfig iconConfig;
	iconConfig.MergeMode = true;
	iconConfig.GlyphMinAdvanceX = 18;
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	ImGuiIO& io = ImGui::GetIO();
	fontSans = io.Fonts->AddFontFromFileTTF("Assets/Roboto-Regular.ttf", 18);
	fontSans = io.Fonts->AddFontFromFileTTF("Assets/fa-solid.ttf", 13, &iconConfig, icon_ranges);
	fontMono = io.Fonts->AddFontFromFileTTF("Assets/JetBrainsMono-Regular.ttf", 17);
	fontTitle = io.Fonts->AddFontFromFileTTF("Assets/Roboto-Bold.ttf", 22);
	iconConfig.GlyphMinAdvanceX = 22;
	fontTitle = io.Fonts->AddFontFromFileTTF("Assets/fa-solid.ttf", 16, &iconConfig, icon_ranges);
	fontHeader = io.Fonts->AddFontFromFileTTF("Assets/Roboto-Bold.ttf", 19);

	// fileDialog icon creation setup
	ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
		GLuint tex;

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		return (void*)tex;
	};
	ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
		GLuint texID = (GLuint)((uintptr_t)tex);
		glDeleteTextures(1, &texID);
	};

	frametimePlotConfig.values.count = FRAMETIME_HISTORY_COUNT;
	frametimePlotConfig.tooltip.show = true;
	frametimePlotConfig.tooltip.format = "%.0s %.2f ms";
	frametimePlotConfig.grid_y.show = true;
	frametimePlotConfig.grid_y.size = 10;
	frametimePlotConfig.grid_y.subticks = 10;
	//frametimePlotConfig.line_thickness = 2.f;
	frametimePlotConfig.frame_size = ImVec2(160, 80);
	frametimePlotConfig.scale.min = 0;
	frametimePlotConfig.scale.max = 1;
}

void SpirmarkUI::Update()
{
	 // update Performance window periodically
	if ((perfDisplayTimer -= core.bencher.GetLastFrameTime().total / 1000) < 0)
	{
		perfDisplayTimer = 0.5f;
		perfDisplayFrameTime = core.bencher.GetLastFrameTime();
		perfDisplayFps = core.bencher.GetFps();
	}
}

void SpirmarkUI::RenderUI()
{
	//ImGui::ShowDemoWindow();

	RenderPopups();

	RenderShaderLoaderWindow();
	RenderBenchmarkWindow();
	RenderSettingsWindow();
	RenderStatsWindow();
	RenderPerfWindow();
}


void SpirmarkUI::RenderPopups()
{
	if (Errors::lastError != Errors::ErrorType::none)
	{
		switch (Errors::lastError)
		{
		case Errors::ErrorType::shaderCompilation:
			errorMsgDisplayed = "Shader compilation failed.\n\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::fileAlreadyOpen:
			errorMsgDisplayed = "Shader already queued for benchmark:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::benchDone:
			errorMsgDisplayed = "Benchmark successful, results saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::benchWrongPath:
			errorMsgDisplayed = "Benchmark successful, could not save results at specified location, results saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::benchCantSave:
			errorMsgDisplayed = "Benchmark failed: could not save results at specified or default location.";
			break;
		case Errors::ErrorType::exportDone:
			errorMsgDisplayed = "Export successful, files saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::exportWrongPath:
			errorMsgDisplayed = "Export done, could not save files at specified location, files saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::exportCantSave:
			errorMsgDisplayed = "Export unsuccessful: could not save files at specified or default location.";
			break;
		case Errors::ErrorType::simDone:
			errorMsgDisplayed = "Similarity measurement successful, files saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::simWrongPath:
			errorMsgDisplayed = "Similarity measurement done, could not save files at specified location, files saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::simCantSave:
			errorMsgDisplayed = "Similarity measurement unsuccessful: could not save files at specified or default location.";
			break;
		case Errors::ErrorType::simImgDone:
			errorMsgDisplayed = "Difference image export successful, files saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::simImgWrongPath:
			errorMsgDisplayed = "Difference image export done, could not save files at specified location, files saved at:\n" + Errors::lastErrorMessage;
			break;
		case Errors::ErrorType::simImgCantSave:
			errorMsgDisplayed = "Difference image export unsuccessful: could not save files at specified or default location.";
			break;

		}
		
		Errors::lastError = Errors::ErrorType::none;
		Errors::lastErrorMessage = std::string();
		ImGui::OpenPopup("Spirmark Message");
	}
	
	ImGui::PushFont(fontTitle);
	if (ImGui::BeginPopupModal("Spirmark Message", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::PushFont(fontSans);
		ImGui::Text(errorMsgDisplayed.c_str());
		ImGui::Separator();

		ImGui::SetCursorPosX(ImGui::GetWindowSize().x * .1f);
		if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.8f, 1.5f * ImGui::GetFontSize())))
			ImGui::CloseCurrentPopup();
		ImGui::SetItemDefaultFocus();
		ImGui::PopFont();
		ImGui::EndPopup();
	}
	ImGui::PopFont();


	// from Shader Loader
	if (ifd::FileDialog::Instance().IsDone("ShaderOpenDialogAssembly")) {
		if (ifd::FileDialog::Instance().HasResult()) {
			std::filesystem::path fp = ifd::FileDialog::Instance().GetResult();
			std::string folder = fp.parent_path().string(), name = fp.filename().string(), path = fp.string();
			IO::saveLocations.shader = folder;
			core.loadedShaders[0] = BenchmarkedShader(name, std::filesystem::path(path));
			core.CompileLoadedShaderToys(false);
		}
		ifd::FileDialog::Instance().Close();
	}

	// from Benchmark
	if (ifd::FileDialog::Instance().IsDone("ShaderOpenDialogBenchmark")) {
		if (ifd::FileDialog::Instance().HasResult())
		{
			auto results = ifd::FileDialog::Instance().GetResults();
			IO::saveLocations.shader = results[0].parent_path();
			for (auto& res : results)
				IO::LoadBenchmarkable(res, true, core.benchmarkedShaderResults, core.benchmarkLength);
		}
		ifd::FileDialog::Instance().Close();
	}

	// from Settings
	if (ifd::FileDialog::Instance().IsDone("DirectoryOpenDialog")) {
		if (ifd::FileDialog::Instance().HasResult()) {
			*pathToBeEdited = ifd::FileDialog::Instance().GetResult();
			*pathToBeEdited = pathToBeEdited->make_preferred();
		}
		ifd::FileDialog::Instance().Close();
	}
}

void SpirmarkUI::RenderShaderLoaderWindow()
{
	if (UIWidgets::BeginWindow(ICON_FA_PALETTE" Shader Loader", fontSans, fontTitle))
	{
		if (core.benchmarkStage && ImGui::IsWindowHovered()) 
			ImGui::SetTooltip("Can't change settings while a benchmark is running.");

		float windowWidth = ImGui::GetWindowContentRegionWidth();

		ImGui::SetCursorPosX(8);
		if (UIWidgets::Button(ICON_FA_FILE_IMPORT" Load", !core.benchmarkStage, ImVec2((windowWidth - 8) / 2, 2 * ImGui::GetFontSize())))
			ifd::FileDialog::Instance().Open("ShaderOpenDialogAssembly", "Open a ShaderToy shader", "GLSL{.frag},.*", false, IO::saveLocations.shader.string());
		ImGui::SameLine();
		ImGui::SetCursorPosX(8 * 2 + (windowWidth - 8) / 2);
		if (UIWidgets::Button(ICON_FA_FILE_EXPORT" Export", core.loadedShaders[0].IsCompiled() && !core.benchmarkStage, ImVec2((windowWidth - 8) / 2, 2 * ImGui::GetFontSize())))
			IO::ExportShaders(core.loadedShaders);

		ImGui::Text(core.loadedShaders[0].GetName().c_str());
		std::string ict = "SpirvMultiOpt iterations: " + std::to_string(core.loadedShaders[0].GetSpirvMultiOptIterUseful());
		ImGui::Text(ict.c_str());

		ImGui::Separator();

		UIWidgets::CenteredText("Optimization", fontHeader);
		for (int i = 0; i < shaderProgramTypeCount; i++)
		{
			UIWidgets::Radio(shaderProgramTypeNames[i], &core.activeShaderProgram, i, !core.benchmarkStage);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip(shaderProgramTypeDescriptions[i]);
		}
	} UIWidgets::EndWindow();
}

void SpirmarkUI::RenderBenchmarkWindow()
{
	if (UIWidgets::BeginWindow(ICON_FA_TACHOMETER_ALT" Benchmark", fontSans, fontTitle))
	{
		float windowWidth = ImGui::GetWindowContentRegionWidth();
		int marginRight = ImGui::CalcTextSize("Model count (~15k tris)").x + 4;

		ImGui::SetCursorPosX(8);
		if (!core.benchmarkStage)
		{
			if (UIWidgets::Button(ICON_FA_PLAY" Start Benchmark", core.benchmarkedShaderResults.size() > 0, ImVec2(windowWidth, 2 * ImGui::GetFontSize()))) core.StartBenchmark();
			ImGui::PushItemWidth(-marginRight);
			ImGui::InputFloat("Length (m)", &core.benchmarkLength);
		}
		else
		{
			if (ImGui::Button(ICON_FA_TIMES" Cancel Benchmark", ImVec2(windowWidth, 2 * ImGui::GetFontSize()))) core.CancelBenchmark();
			float shaderTime = core.benchmarkLength * 60 / (core.benchmarkMode != MultiModel ? core.benchmarkedShaderResults.size() : 1); // multi model means we are benchmarking all at once ("only one shader")
			int timeRem = core.bencher.GetRemainingBenchmarkTime(); // time remaining from current benchmark
			timeRem += core.benchmarkLength * 60 - (core.benchmarkedShader - 1) * shaderTime; // all time - shaders we are done with
			timeRem -= (core.benchmarkStage) * shaderTime / shaderProgramTypeCount; // stages we are done with
			std::stringstream benchmarkRemainingText;
			benchmarkRemainingText << "Benchmark running... (" << timeRem / 3600 << " h " << (timeRem / 60) % 60 << " m " << timeRem % 60 << " s left)";
			ImGui::Text(benchmarkRemainingText.str().c_str());
		}

		ImGui::PushItemWidth(-marginRight);

		UIWidgets::SliderInt("Model count (~15k tris)", &core.suzanneCount, 1, 50, !core.benchmarkStage);
		UIWidgets::Checkbox("Export benchmarked shaders", &core.exportBenchmarkedShaders, !core.benchmarkStage);

		for (int i = 0; i < BenchmarkMode::benchmarkModeCount; i++)
		{
			std::string benchmarkModeFullName = benchmarkModeIcons[i] + " " + benchmarkModeNames[i];
            UIWidgets::Radio(benchmarkModeFullName.c_str(), &core.benchmarkMode, i, !core.benchmarkStage);
		}
		
		ImGui::Separator();
		UIWidgets::CenteredText("Benchmarked shaders", fontHeader);

		if (!core.benchmarkStage)
		{
			ImGui::SetCursorPosX(8);
			if (ImGui::Button(ICON_FA_PLUS, ImVec2(windowWidth, 1.5f * ImGui::GetFontSize())))
				ifd::FileDialog::Instance().Open("ShaderOpenDialogBenchmark", "Open ShaderToy shaders or spirmark config files",
					"GLSL or Spirmark config{.frag,.spirmark},.*", true, IO::saveLocations.shader.string());
		}

		for (int i = 0; i < core.benchmarkedShaderResults.size(); i++)
		{
			if (i < core.benchmarkedShader - 1) ImGui::PushStyleColor(ImGuiCol_Text, uiColors[accentDark]);
			else if (i == core.benchmarkedShader - 1) ImGui::PushStyleColor(ImGuiCol_Text, uiColors[accent]);
			ImGui::Text(core.benchmarkedShaderResults[i].name.c_str());
			if (i <= core.benchmarkedShader - 1) ImGui::PopStyleColor();

			if (!core.benchmarkStage)
			{
				ImGui::SameLine(windowWidth - 28 + 8);
				std::stringstream s; s << ICON_FA_TIMES"##" << i;
				if (ImGui::Button(s.str().c_str(), ImVec2(28, 0)))
				{
					core.benchmarkedShaderResults.erase(core.benchmarkedShaderResults.begin() + i);
					i--;
				}
			}
		}
	} UIWidgets::EndWindow();
}

void SpirmarkUI::RenderSettingsWindow()
{
	if (UIWidgets::BeginWindow(ICON_FA_COG" Settings", fontSans, fontTitle))
	{
		if (ImGui::Button(ICON_FA_FOLDER "##benchreslocbut"))
		{
			pathToBeEdited = &IO::saveLocations.benchResult;
			ifd::FileDialog::Instance().Open("DirectoryOpenDialog", "Open a directory", "", false, pathToBeEdited->string());
		}
		ImGui::SameLine();
		std::string resultLoc = ICON_FA_FILE_EXCEL" Benchmark result location: \n" + IO::saveLocations.benchResult.string();
		ImGui::Text(resultLoc.c_str());

		if (ImGui::Button(ICON_FA_FOLDER "##exportlocbut"))
		{
			pathToBeEdited = &IO::saveLocations.shaderExport;
			ifd::FileDialog::Instance().Open("DirectoryOpenDialog", "Open a directory", "", false, pathToBeEdited->string());
		}
		ImGui::SameLine();
		std::string exportLoc = ICON_FA_FILE_CODE" Shader export location: \n" + IO::saveLocations.shaderExport.string();
		ImGui::Text(exportLoc.c_str());

		if (ImGui::Button(ICON_FA_FOLDER "##simlocbut"))
		{
			pathToBeEdited = &IO::saveLocations.simResult;
			ifd::FileDialog::Instance().Open("DirectoryOpenDialog", "Open a directory", "", false, pathToBeEdited->string());
		}
		ImGui::SameLine();
		std::string simLoc = ICON_FA_FILE_EXCEL" Similarity results location: \n" + IO::saveLocations.simResult.string();
		ImGui::Text(simLoc.c_str());

		if (ImGui::Button(ICON_FA_FOLDER "##simimglocbut"))
		{
			pathToBeEdited = &IO::saveLocations.simImg;
			ifd::FileDialog::Instance().Open("DirectoryOpenDialog", "Open a directory", "", false, pathToBeEdited->string());
		}
		ImGui::SameLine();
		std::string simImgLoc = ICON_FA_FILE_IMAGE" Similarity images location: \n" + IO::saveLocations.simImg.string();
		ImGui::Text(simImgLoc.c_str());

		int spirvMultiOptIterCountMaxTemp = core.spirvMultiOptIterCountMax;
		ImGui::PushItemWidth(85);
		UIWidgets::InputInt(ICON_FA_REDO" Max spirvMultiOpt iterations", &spirvMultiOptIterCountMaxTemp, !core.benchmarkStage, 1, 5);
		core.spirvMultiOptIterCountMax = std::min(std::max(spirvMultiOptIterCountMaxTemp, 1), 99);

		UIWidgets::Checkbox(ICON_FA_DESKTOP" VSync", core.vsync, !core.benchmarkStage);
	} UIWidgets::EndWindow();
}

void SpirmarkUI::RenderStatsWindow()
{
	if (UIWidgets::BeginWindow(ICON_FA_TABLE" Shader stats", fontSans, fontTitle))
	{
		// compact
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));

		ImGui::BeginTable("stats", 15, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable);

		// header
		ImGui::TableSetupColumn("*");
		for (const char* h : Spirver::shaderStatNames) ImGui::TableSetupColumn(h);
		ImGui::TableHeadersRow();

		for (int i = 0; i < shaderProgramTypeCount; i++)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text(shaderProgramTypeNames[i]);
			for (int j = 0; j < Spirver::shaderStatTypesCount; j++)
			{
				ImGui::TableNextColumn();
				ImGui::Text("%i", core.loadedShaders[0].GetShaderStats()[i].stats[j]);
			}
		}

		ImGui::EndTable();
		ImGui::PopStyleVar(2);
	} UIWidgets::EndWindow();
}

void SpirmarkUI::RenderPerfWindow()
{
	
	if (UIWidgets::BeginWindow(ICON_FA_CHART_BAR" Performance", fontSans, fontTitle, ImGuiWindowFlags_NoDecoration))
	{
		ImGui::SetWindowSize(ImVec2(175, 185));
		UIWidgets::CenteredText(ICON_FA_CHART_BAR" Performance", fontTitle);
		std::stringstream s;
		s << std::setprecision(2) << std::fixed
			<< "Frametime: " << perfDisplayFrameTime.total << " ms" << std::endl
			<< "GPU: " << perfDisplayFrameTime.gpu << " ms" << std::endl
			<< "FPS: " << perfDisplayFps << std::endl;
		ImGui::PushFont(fontMono);
		ImGui::Text(s.str().c_str());
		ImGui::PopFont();
		
		float graphData[FRAMETIME_HISTORY_COUNT] = { 0 };
		const std::vector<FrameTime>& data = core.bencher.GetFrameTimeHistory();
		for (int i = 0; i < data.size(); i++) graphData[i] = data[i].gpu;
		frametimePlotConfig.values.ys = graphData;
		frametimePlotConfig.scale.min = std::min(frametimePlotConfig.scale.min, *std::min_element(std::begin(graphData), std::end(graphData)));
		frametimePlotConfig.scale.max = std::max(frametimePlotConfig.scale.max, *std::max_element(std::begin(graphData), std::end(graphData)));
		
		ImGui::Plot("Frametimes", frametimePlotConfig);
	} UIWidgets::EndWindow();
}
