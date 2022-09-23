#pragma once

#include <Windows.h>

#include "imgui.h"
#include "Overlay.h"

namespace ProcessPriority {

static int current_priority = HIGH_PRIORITY_CLASS;

inline void init()
{
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    spdlog::trace("Set process priority to HIGH_PRIORITY_CLASS");

    Overlay::AddOverlayElem([](bool window_has_focus, ImGuiID dockspace_id) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::Begin("Process Priority");
        ImGui::Text("Might help with input-lag or bad game performance");
        if (ImGui::RadioButton("Realtime", current_priority == REALTIME_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
            current_priority = REALTIME_PRIORITY_CLASS;
            spdlog::trace("Set process priority to REALTIME_PRIORITY_CLASS");
        }
        if (ImGui::RadioButton("High", current_priority == HIGH_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
            current_priority = HIGH_PRIORITY_CLASS;
            spdlog::trace("Set process priority to HIGH_PRIORITY_CLASS");
        }
        if (ImGui::RadioButton("Above Normal", current_priority == ABOVE_NORMAL_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
            current_priority = ABOVE_NORMAL_PRIORITY_CLASS;
            spdlog::trace("Set process priority to ABOVE_NORMAL_PRIORITY_CLASS");
        }
        if (ImGui::RadioButton("Normal", current_priority == NORMAL_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
            current_priority = NORMAL_PRIORITY_CLASS;
            spdlog::trace("Set process priority to NORMAL_PRIORITY_CLASS");
        }
        if (ImGui::RadioButton("Below Normal", current_priority == BELOW_NORMAL_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
            current_priority = BELOW_NORMAL_PRIORITY_CLASS;
            spdlog::trace("Set process priority to BELOW_NORMAL_PRIORITY_CLASS");
        }
        if (ImGui::RadioButton("Low", current_priority == IDLE_PRIORITY_CLASS)) {
            SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
            current_priority = IDLE_PRIORITY_CLASS;
            spdlog::trace("Set process priority to IDLE_PRIORITY_CLASS");
        }
        ImGui::End();
    });
}

} // namespace ProcessPriority
