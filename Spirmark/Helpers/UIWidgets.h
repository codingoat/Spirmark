#pragma once

#include <imgui/imgui.h>

namespace UIWidgets
{

bool Button(const char* label, bool interactable = true, ImVec2 size = ImVec2(0, 0), ImGuiButtonFlags flags = ImGuiButtonFlags_None);
bool Radio(const char* label, int* v, int v_button, bool interactable = true);
bool Checkbox(const char* label, bool* value, bool interactable = true);
bool InputInt(const char* label, int* v, bool interactable = true, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
bool InputScalar(const char* label, ImGuiDataType data_type, void* p_data, bool intearctable = true, const void* p_step = (const void*)0, const void* p_step_fast = (const void*)0, const char* format = (const char*)0, ImGuiInputTextFlags flags = 0);
bool SliderInt(const char* label, int* v, int v_min, int v_max, bool interactable = true, const char* format = "%d", ImGuiSliderFlags flags = 0);

void CenteredText(const char* label, ImFont* font = nullptr);
bool BeginWindow(const char* label, ImFont* textFont = nullptr, ImFont* titleFont = nullptr, ImGuiWindowFlags flags = ImGuiWindowFlags_None, bool* p_open = nullptr);
void EndWindow();


void ApplyInactiveStyle();
void RemoveInactiveStyle();

}