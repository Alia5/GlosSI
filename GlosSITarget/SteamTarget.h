/*
Copyright 2021-2023 Peter Repukat - FlatspotSoftware

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
#define WIN32_LEAN_AND_MEAN

#include "SteamOverlayDetector.h"

#include "TargetWindow.h"

#ifdef _WIN32
#include "../common/HidHide.h"
#include "InputRedirector.h"
#include <subhook.h>
#endif

#include <filesystem>

#include "AppLauncher.h"
#include "Overlay.h"
#include "HttpServer.h"

#include "../common/steam_util.h"

namespace Tray {
class Tray;
}
class SteamTarget {
  public:
    explicit SteamTarget();
    int run();

  private:
    void onOverlayChanged(bool overlay_open);
    void toggleGlossiOverlay();
    void focusWindow(WindowHandle hndl);

    std::filesystem::path steam_path_ = util::steam::getSteamPath();
    std::wstring steam_user_id_ = util::steam::getSteamUserId();

    bool steam_overlay_present_ = false;

    // Keep controllerConfig even is window is switched.
    // On Windoze hooking "GetForeGroundWindow" is enough;
    void keepControllerConfig(bool keep);

#ifdef _WIN32
    static HWND keepFgWindowHookFn();
    static inline subhook::Hook getFgWinHook;
    static inline std::vector<HWND> force_config_hwnds_ = {};
    static inline HWND last_real_hwnd_ = nullptr;
#endif

    std::unique_ptr<Tray::Tray> createTrayMenu();

    /*
     * Run once per frame
     * detects steam configured overlay hotkey, and simulates key presses to window
     *
     * actually opens the overlay, even if window is not currently focused.
     */
    void overlayHotkeyWorkaround();

    bool run_ = false;
    std::vector<std::string> overlay_hotkey_ = util::steam::getOverlayHotkey(steam_path_, steam_user_id_);

#ifdef _WIN32
    HidHide hidhide_;
    InputRedirector input_redirector_;
#endif
    TargetWindow window_;
    std::weak_ptr<Overlay> overlay_;
    SteamOverlayDetector detector_;
    AppLauncher launcher_;
    HttpServer server_;
    WindowHandle last_foreground_window_ = nullptr;
    static inline WindowHandle target_window_handle_ = nullptr;

    sf::Clock overlay_trigger_clock_;
    float overlay_trigger_max_seconds_ = 2.5;
    bool overlay_trigger_flag_ = false;

    bool delayed_shutdown_ = false;
    sf::Clock delay_shutdown_clock_;
};
