#pragma once

#include <filesystem>
#include <string>

#include "DllInjector.h"
#include "Overlay.h"

namespace UWPOverlayEnabler {

namespace internal {

inline std::filesystem::path EnablerPath()
{
    wchar_t buff[MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), buff, MAX_PATH);
    const std::wstring path(buff);
    return path.substr(0, 1 + path.find_last_of(L'\\')) + L"UWPOverlayEnablerDLL.dll";
}


} // namespace internal

inline void EnableUwpOverlay()
{
    const auto enabler_path = internal::EnablerPath();
    DllInjector::injectDllInto(enabler_path, L"explorer.exe");
}

inline void AddUwpOverlayOvWidget()
{
    Overlay::AddOverlayElem([](bool window_has_focus, ImGuiID dockspace_id) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::Begin("UWP-Overlay");
        ImGui::Text("To enable the overlay on top of \"fullscreen\" UWP-Apps,");
        ImGui::Text("a .dll has to be injected into explorer.exe");
        ImGui::Spacing();
        ImGui::Text("This method uses undocumented windows functions");
        ImGui::Text("and might cause issues.");
        ImGui::Spacing();
        if (ImGui::Button("Enable")) {
            EnableUwpOverlay();
        }
        ImGui::Text("If the overlay isn't working right away:");
        ImGui::Text("try opening Windows start menu, as this triggers the hook");
        ImGui::End();
    });
}

} // namespace UWPOverlayEnabler