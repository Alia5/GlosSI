/*
Copyright 2021 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once

#include "OverlayDetector.h"

#include "TargetWindow.h"

#ifdef _WIN32
#include "HidHide.h"
#include "InputRedirector.h"
#endif

#include <filesystem>

class SteamTarget {
  public:
    explicit SteamTarget(int argc, char* argv[]);
    int run();

  private:
    void onOverlayChanged(bool overlay_open);
    void focusWindow(WindowHandle hndl);
    std::filesystem::path getSteamPath() const;
    std::wstring getSteamUserId() const;

    std::filesystem::path steam_path_ = getSteamPath();
    std::wstring steam_user_id_ = getSteamUserId();

    std::vector<std::string> getOverlayHotkey();
    std::vector<std::string> getScreenshotHotkey();

    // Keep controllerConfig even is window is switched.
    // On Windoze hooking "GetForeGroundWindow" is enough;
    void keepControllerConfig(bool keep);
    /*
     * Run once per frame
     * detects steam configured overlay hotkey, and simulates key presses to window
     *
     * actually opens the overlay, even if window is not currently focused.
     */
    void overlayHotkeyWorkaround();

    bool run_ = false;
    std::vector<std::string> overlay_hotkey_ = getOverlayHotkey();

#ifdef _WIN32
    HidHide hidhide_;
    InputRedirector input_redirector_;
#endif
    TargetWindow window_;
    OverlayDetector detector_;
    WindowHandle last_foreground_window_ = nullptr;
    WindowHandle target_window_handle_;

    static constexpr std::wstring_view user_data_path_ = L"/userdata/";
    static constexpr std::wstring_view config_file_name_ = L"/config/localconfig.vdf";
    static constexpr std::string_view overlay_hotkey_name_ = "InGameOverlayShortcutKey ";
    static constexpr std::string_view screenshot_hotkey_name_ = "InGameOverlayScreenshotHotKey ";
};
