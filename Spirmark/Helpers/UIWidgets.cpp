#include <UIWidgets.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui/imgui_internal.h>
using namespace ImGui;

bool UIWidgets::Button(const char* label, bool interactable, ImVec2 size, ImGuiButtonFlags flags)
{
    // implementation based on imgui_widgets.cpp
	if(!interactable) ApplyInactiveStyle();

    // layout
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 actualSize = CalcItemSize(size, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, pos + actualSize);
    ItemSize(actualSize, style.FramePadding.y);
    if (!ItemAdd(bb, id))
    {
        if (!interactable) RemoveInactiveStyle();
        return false;
    }

    // behaviour
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = GetColorU32(interactable ? (held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    if (!interactable) RemoveInactiveStyle();

    return interactable && pressed;
}

bool UIWidgets::Radio(const char* label, int* v, int v_button, bool interactable)
{
    // implementation based on imgui_widgets.cpp
    if (!interactable) ApplyInactiveStyle();

    bool active = *v == v_button;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        if (!interactable) RemoveInactiveStyle();
        return false;
    }

    ImVec2 center = check_bb.GetCenter();
    center.x = IM_ROUND(center.x);
    center.y = IM_ROUND(center.y);
    const float radius = (square_sz - 1.0f) * 0.5f;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        MarkItemEdited(id);

    RenderNavHighlight(total_bb, id);
    window->DrawList->AddCircleFilled(center, radius, GetColorU32(interactable ? (held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg : ImGuiCol_FrameBg), 16);
    if (active)
    {
        const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
        window->DrawList->AddCircleFilled(center, radius - pad, GetColorU32(ImGuiCol_CheckMark), 16);
    }

    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddCircle(center + ImVec2(1, 1), radius, GetColorU32(ImGuiCol_BorderShadow), 16, style.FrameBorderSize);
        window->DrawList->AddCircle(center, radius, GetColorU32(ImGuiCol_Border), 16, style.FrameBorderSize);
    }

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
    if (g.LogEnabled)
        LogRenderedText(&label_pos, active ? "(x)" : "( )");
    if (label_size.x > 0.0f)
        RenderText(label_pos, label);


    if (!interactable) RemoveInactiveStyle();

    bool actuallyPressed = pressed && interactable;
    if(actuallyPressed) *v = v_button;
    return actuallyPressed;
}

bool UIWidgets::Checkbox(const char* label, bool* value, bool interactable)
{
    // implementation based on imgui_widgets.cpp
    if (!interactable) ApplyInactiveStyle();

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        if (!interactable) RemoveInactiveStyle();
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (interactable && pressed)
    {
        *value = !(*value);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    RenderNavHighlight(total_bb, id);
    RenderFrame(check_bb.Min, check_bb.Max, GetColorU32(interactable ? (held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg : ImGuiCol_FrameBg), true, style.FrameRounding);
    ImU32 check_col = GetColorU32(ImGuiCol_CheckMark);
    if (*value)
    {
        const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
        RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
    }

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
    if (label_size.x > 0.0f)
        RenderText(label_pos, label);

    if (!interactable) RemoveInactiveStyle();

    return interactable && pressed;
}

bool UIWidgets::InputInt(const char* label, int* v, bool interactable, int step, int step_fast, ImGuiInputTextFlags flags)
{
    // implementation based on imgui_widgets.cpp
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return UIWidgets::InputScalar(label, ImGuiDataType_S32, (void*)v, interactable, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
    return false;
}

bool UIWidgets::InputScalar(const char* label, ImGuiDataType data_type, void* p_data, bool interactable, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags)
{
    // implementation based on imgui_widgets.cpp
    if (!interactable) ApplyInactiveStyle();

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    char buf[64];
    DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);

    bool value_changed = false;
    if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
        flags |= ImGuiInputTextFlags_CharsDecimal;
    flags |= ImGuiInputTextFlags_AutoSelectAll;
    flags |= ImGuiInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.
    if (!interactable) flags |= ImGuiInputTextFlags_ReadOnly;

    if (p_step != NULL)
    {
        const float button_size = GetFrameHeight();

        BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
        PushID(label);
        SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
        if (InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
            value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);

        // Step buttons
        const ImVec2 backup_frame_padding = style.FramePadding;
        style.FramePadding.x = style.FramePadding.y;
        ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
        if (flags & ImGuiInputTextFlags_ReadOnly)
            button_flags |= ImGuiButtonFlags_Disabled;
        SameLine(0, style.ItemInnerSpacing.x);
        if (UIWidgets::Button("-", interactable, ImVec2(button_size, button_size), button_flags))
        {
            DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
            value_changed = true;
        }
        SameLine(0, style.ItemInnerSpacing.x);
        if (UIWidgets::Button("+", interactable, ImVec2(button_size, button_size), button_flags))
        {
            DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
            value_changed = true;
        }

        const char* label_end = FindRenderedTextEnd(label);
        if (label != label_end)
        {
            SameLine(0, style.ItemInnerSpacing.x);
            TextEx(label, label_end);
        }
        style.FramePadding = backup_frame_padding;

        PopID();
        EndGroup();
    }
    else
    {
        if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
            value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
    }
    if (value_changed)
        MarkItemEdited(window->DC.LastItemId);

    if (!interactable) RemoveInactiveStyle();

    return interactable && value_changed;
}

bool UIWidgets::SliderInt(const char* label, int* v, int v_min, int v_max, bool interactable, const char* format, ImGuiSliderFlags flags)
{
    // implementation based on imgui_widgets.cpp
    int temp = *v;
    int* real_v;
    if (!interactable)
    {
        ApplyInactiveStyle();
        real_v = &temp;
    }
    else
    {
        real_v = v;
    }

    bool ret = ImGui::SliderInt(label, real_v, v_min, v_max, format, flags);

    if (!interactable) RemoveInactiveStyle();

    return ret && interactable;
}

void UIWidgets::CenteredText(const char* label, ImFont* font)
{
    if (font) ImGui::PushFont(font);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(label).x / 2);
    ImGui::Text(label);
    if (font) ImGui::PopFont();
}

bool UIWidgets::BeginWindow(const char* label, ImFont* textFont, ImFont* titleFont, ImGuiWindowFlags flags, bool* p_open)
{
    // this is needed because font stack is per-window

    ImGui::PushFont(titleFont);
    if (!ImGui::Begin(label, p_open, flags))
    {
        ImGui::PushFont(textFont);
        return false;
    }
    ImGui::PushFont(textFont);
    return true;
}

void UIWidgets::EndWindow()
{
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopFont();
}

void UIWidgets::ApplyInactiveStyle()
{
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void UIWidgets::RemoveInactiveStyle()
{
	ImGui::PopStyleVar();
}
