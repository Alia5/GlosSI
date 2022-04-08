#pragma once

#include <Windows.h>

#include "imgui.h"
#include "Overlay.h"

namespace ProcessPriority {

static int current_priority = HIGH_PRIORITY_CLASS;

inline void init()
{
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    Overlay::AddOverlayElem([](bool window_has_focus) {
        ImGui::SetNextWindowPos({913, 418}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints({170, 325}, {1000, 1000});
        ImGui::Begin("Process Priority");
        ImGui::Text("Might help with input-lag or bad game performance");
        if (ImGui::RadioButton("Realtime", current_priority == REALTIME_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
            current_priority = REALTIME_PRIORITY_CLASS;
        }
        if (ImGui::RadioButton("High", current_priority == HIGH_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
            current_priority = HIGH_PRIORITY_CLASS;
        }
        if (ImGui::RadioButton("Above Normal", current_priority == ABOVE_NORMAL_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
            current_priority = ABOVE_NORMAL_PRIORITY_CLASS;
        }
        if (ImGui::RadioButton("Normal", current_priority == NORMAL_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
            current_priority = NORMAL_PRIORITY_CLASS;
        }
        if (ImGui::RadioButton("Below Normal", current_priority == BELOW_NORMAL_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
            current_priority = BELOW_NORMAL_PRIORITY_CLASS;
        }
        if (ImGui::RadioButton("Low", current_priority == IDLE_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
            current_priority = IDLE_PRIORITY_CLASS;
        }
        ImGui::End();
    });
}

} // namespace ProcessPriority
