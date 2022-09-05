/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

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
#include "SteamTarget.h"

#include "Settings.h"
#include "steam_sf_keymap.h"

#include <SFML/Window/Keyboard.hpp>
#include <WinReg/WinReg.hpp>
#include <numeric>
#include <regex>
#include <spdlog/spdlog.h>
#include <vdf_parser.hpp>

#ifdef _WIN32
#include "UWPOverlayEnabler.h"
#include <tray.hpp>
#endif

SteamTarget::SteamTarget()
    : window_(
          [this] { run_ = false; },
          [this] { toggleGlossiOverlay(); },
          getScreenshotHotkey(),
          [this]() {
              target_window_handle_ = window_.getSystemHandle();
              overlay_ = window_.getOverlay();
          }),
      overlay_(window_.getOverlay()),
      detector_([this](bool overlay_open) { onOverlayChanged(overlay_open); }),
      launcher_(force_config_hwnds_, [this] {
          delayed_shutdown_ = true;
          delay_shutdown_clock_.restart();
      })
{
    target_window_handle_ = window_.getSystemHandle();
#ifdef _WIN32
    if (Settings::launch.isUWP) {
        UWPOverlayEnabler::EnableUwpOverlay();
    }
    else {
        UWPOverlayEnabler::AddUwpOverlayOvWidget();
    }
#endif
}

int SteamTarget::run()
{
    if (!SteamOverlayDetector::IsSteamInjected()) {
        spdlog::warn("Steam-overlay not detected. Showing GlosSI-overlay!\n\
Application will not function!");
        window_.setClickThrough(false);
        if (!overlay_.expired())
            overlay_.lock()->setEnabled(true);
        steam_overlay_present_ = false;
    }
    else {
        spdlog::info("Steam-overlay detected.");
        spdlog::warn("Double press Steam- overlay key(s)/Controller button to show GlosSI-overlay"); // Just to color output and really get users attention
        window_.setClickThrough(true);
        if (!overlay_.expired())
            overlay_.lock()->setEnabled(false);
        steam_overlay_present_ = true;
    }
    getXBCRebindingEnabled();

    run_ = true;

#ifdef _WIN32
    hidhide_.hideDevices(steam_path_);
    input_redirector_.run();
#endif

    if (Settings::launch.launch) {
        launcher_.launchApp(Settings::launch.launchPath, Settings::launch.launchAppArgs);
    }

    keepControllerConfig(true);

#ifdef _WIN32
    HICON icon = 0;
    TCHAR path[MAX_PATH];
    GetModuleFileName(nullptr, path, MAX_PATH);
    icon = (HICON)LoadImage(
        0,
        path,
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_LOADFROMFILE | LR_LOADMAP3DCOLORS);
    if (!icon) {
        ExtractIconEx(path, 0, &icon, nullptr, 1);
    }
    Tray::Tray tray{"GlosSITarget", icon};
#else
    Tray::Tray tray{"GlosSITarget", "ico.png"};
#endif

    tray.addEntry(Tray::Button{
        "Quit", [this, &tray]() {
            run_ = false;
        }});

    while (run_) {
        detector_.update();
        overlayHotkeyWorkaround();
        window_.update();
        // Wait on shutdown; User might get confused if window closes to fast if anything with launchApp get's borked.
        if (delayed_shutdown_) {
            if (delay_shutdown_clock_.getElapsedTime().asSeconds() >= 3) {
                run_ = false;
            }
        }
        else {
            launcher_.update();
        }
    }
    tray.exit();

#ifdef _WIN32
    input_redirector_.stop();
    hidhide_.disableHidHide();
#endif
    launcher_.close();
    return 0;
}

void SteamTarget::onOverlayChanged(bool overlay_open)
{
    if (overlay_open) {
        focusWindow(target_window_handle_);
        window_.setClickThrough(!overlay_open);
    }
    else {
        if (!(overlay_.expired() ? false : overlay_.lock()->isEnabled())) {
            window_.setClickThrough(!overlay_open);
            focusWindow(last_foreground_window_);
        }
    }
    if (!overlay_trigger_flag_) {
        overlay_trigger_flag_ = true;
        overlay_trigger_clock_.restart();
    }
    else {
        if (overlay_trigger_clock_.getElapsedTime().asSeconds() <= overlay_trigger_max_seconds_) {
            toggleGlossiOverlay();
        }
        overlay_trigger_flag_ = false;
    }
}

void SteamTarget::toggleGlossiOverlay()
{
    if (overlay_.expired()) {
        return;
    }
    const auto ov_opened = overlay_.lock()->toggle();
    window_.setClickThrough(!ov_opened);
    if (ov_opened) {
        spdlog::debug("Opened GlosSI-overlay");
        focusWindow(target_window_handle_);
    }
    else {
        focusWindow(last_foreground_window_);
        spdlog::debug("Closed GlosSI-overlay");
    }
}

void SteamTarget::focusWindow(WindowHandle hndl)
{
#ifdef _WIN32
    if (hndl == target_window_handle_) {
        spdlog::debug("Bring own window to foreground");
    }
    else {
        spdlog::debug("Bring window \"{:#x}\" to foreground", reinterpret_cast<uint64_t>(hndl));
    }

    keepControllerConfig(false); // unhook GetForegroundWindow
    const auto current_fgw = GetForegroundWindow();
    if (current_fgw != target_window_handle_) {
        last_foreground_window_ = current_fgw;
    }
    const auto fg_thread = GetWindowThreadProcessId(current_fgw, nullptr);

    keepControllerConfig(true); // re-hook GetForegroundWindow

    // lot's of ways actually bringing our window to foreground...
    const auto current_thread = GetCurrentThreadId();
    AttachThreadInput(current_thread, fg_thread, TRUE);

    SetForegroundWindow(hndl);
    SetCapture(hndl);
    SetFocus(hndl);
    SetActiveWindow(hndl);
    EnableWindow(hndl, TRUE);

    AttachThreadInput(current_thread, fg_thread, FALSE);

    //try to forcefully set foreground window at least a few times
    sf::Clock clock;
    while (!SetForegroundWindow(hndl) && clock.getElapsedTime().asMilliseconds() < 20) {
        SetActiveWindow(hndl);
        Sleep(1);
    }

#endif
}

std::filesystem::path SteamTarget::getSteamPath() const
{
#ifdef _WIN32
    try {
        // TODO: check if keys/value exist
        // steam should always be open and have written reg values...
        winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam"};
        const auto res = key.GetStringValue(L"SteamPath");
        spdlog::info(L"Detected Steam Path: {}", res);
        return res;
    }
    catch (const winreg::RegException& e) {
        spdlog::error("Couldn't get Steam path from Registry; {}", e.what());
    }
    return L"";
#else
    return L""; // TODO
#endif
}

std::wstring SteamTarget::getSteamUserId() const
{
#ifdef _WIN32
    try {
        // TODO: check if keys/value exist
        // steam should always be open and have written reg values...
        winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess"};
        const auto res = std::to_wstring(key.GetDwordValue(L"ActiveUser"));
        spdlog::info(L"Detected Steam UserId: {}", res);
        return res;
    }
    catch (const winreg::RegException& e) {
        spdlog::error("Couldn't get Steam path from Registry; {}", e.what());
    }
    return L"";
#else
    return L""; // TODO
#endif
}

std::vector<std::string> SteamTarget::getOverlayHotkey()
{
    const auto config_path = std::wstring(steam_path_) + std::wstring(user_data_path_) + steam_user_id_ + std::wstring(config_file_name_);
    if (!std::filesystem::exists(config_path)) {
        spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
        return {"Shift", "KEY_TAB"}; // default
    }
    std::ifstream config_file(config_path);
    auto root = tyti::vdf::read(config_file);

    std::shared_ptr<tyti::vdf::basic_object<char>> children = root.childs["system"];
    if (!children || children->attribs.empty() || !children->attribs.contains("InGameOverlayShortcutKey")) {
        spdlog::warn("Couldn't detect overlay hotkey, using default: Shift+Tab");
        return {"Shift", "KEY_TAB"}; // default
    }
    auto hotkeys = children->attribs.at("InGameOverlayShortcutKey");

    // has anyone more than 4 keys to open overlay?!
    std::smatch m;
    if (!std::regex_match(hotkeys, m, std::regex(R"((\w*)\s*(\w*)\s*(\w*)\s*(\w*))"))) {
        spdlog::warn("Couldn't detect overlay hotkey, using default: Shift+Tab");
        return {"Shift", "KEY_TAB"}; // default
    }

    std::vector<std::string> res;
    for (auto i = 1; i < m.size(); i++) {
        const auto s = std::string(m[i]);
        if (!s.empty()) {
            res.push_back(s);
        }
    }
    if (res.empty()) {
        spdlog::warn("Couldn't detect overlay hotkey, using default: Shift+Tab");
        return {"Shift", "KEY_TAB"}; // default
    }
    spdlog::info("Detected Overlay hotkey(s): {}", std::accumulate(
                                                       res.begin() + 1, res.end(), res[0],
                                                       [](auto acc, const auto curr) { return acc += "+" + curr; }));
    return res;
}

std::vector<std::string> SteamTarget::getScreenshotHotkey()
{
    const auto config_path = std::wstring(steam_path_) + std::wstring(user_data_path_) + steam_user_id_ + std::wstring(config_file_name_);
    if (!std::filesystem::exists(config_path)) {
        spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
        return {"KEY_F12"}; //default
    }
    std::ifstream config_file(config_path);
    auto root = tyti::vdf::read(config_file);

    std::shared_ptr<tyti::vdf::basic_object<char>> children = root.childs["system"];
    if (!children || children->attribs.empty() || !children->attribs.contains("InGameOverlayScreenshotHotKey")) {
        spdlog::warn("Couldn't detect overlay hotkey, using default: F12");
        return {"KEY_F12"}; //default
    }
    auto hotkeys = children->attribs.at("InGameOverlayScreenshotHotKey");

    // has anyone more than 4 keys to screenshot?!
    std::smatch m;
    if (!std::regex_match(hotkeys, m, std::regex(R"((\w*)\s*(\w*)\s*(\w*)\s*(\w*))"))) {
        spdlog::warn("Couldn't detect overlay hotkey, using default: F12");
        return {"KEY_F12"}; //default
    }

    std::vector<std::string> res;
    for (auto i = 1; i < m.size(); i++) {
        const auto s = std::string(m[i]);
        if (!s.empty()) {
            res.push_back(s);
        }
    }
    if (res.empty()) {
        spdlog::warn("Couldn't detect overlay hotkey, using default: F12");
        return {"KEY_F12"}; //default
    }
    spdlog::info("Detected screenshot hotkey(s): {}", std::accumulate(
                                                          res.begin() + 1, res.end(), res[0],
                                                          [](auto acc, const auto curr) { return acc += "+" + curr; }));
    return res;
}

bool SteamTarget::getXBCRebindingEnabled()
{
    const auto config_path = std::wstring(steam_path_) + std::wstring(user_data_path_) + steam_user_id_ + std::wstring(config_file_name_);
    if (!std::filesystem::exists(config_path)) {
        spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
        return false;
    }
    std::ifstream config_file(config_path);
    auto root = tyti::vdf::read(config_file);

    if (root.attribs.empty() || !root.attribs.contains("SteamController_XBoxSupport")) {
        spdlog::warn("\"Xbox Configuration Support\" is disabled in Steam. This may cause doubled Inputs!");
        return false;
    }
    auto xbsup = root.attribs.at("SteamController_XBoxSupport");
    if (xbsup != "1") {
        spdlog::warn("\"Xbox Configuration Support\" is disabled in Steam. This may cause doubled Inputs!");
    }
    return xbsup == "1";
}

/*
 * The "magic" that keeps a controller-config forced (without hooking into Steam)
 *
 * Hook into own process and detour "GetForegroundWindow"
 * Deatour function always returns HWND of own application window
 * Steam now doesn't detect application changes and keeps the game-specific input config without reverting to desktop-conf
 */
void SteamTarget::keepControllerConfig(bool keep)
{
#ifdef _WIN32
    if (keep && !getFgWinHook.IsInstalled()) {
        spdlog::debug("Hooking GetForegroundWindow (in own process)");
        getFgWinHook.Install(&GetForegroundWindow, &keepFgWindowHookFn, subhook::HookFlags::HookFlag64BitOffset);
        if (!getFgWinHook.IsInstalled()) {
            spdlog::error("Couldn't install GetForegroundWindow hook!");
        }
    }
    else if (!keep && getFgWinHook.IsInstalled()) {
        spdlog::debug("Un-Hooking GetForegroundWindow (in own process)");
        getFgWinHook.Remove();
        if (getFgWinHook.IsInstalled()) {
            spdlog::error("Couldn't un-install GetForegroundWindow hook!");
        }
    }
#endif
}

#ifdef _WIN32
HWND SteamTarget::keepFgWindowHookFn()
{
    if (!Settings::controller.allowDesktopConfig || !Settings::launch.launch) {
        return target_window_handle_;
    }
    subhook::ScopedHookRemove remove(&getFgWinHook);
    HWND real_fg_win = GetForegroundWindow();
    if (real_fg_win == nullptr) {
        return target_window_handle_;
    }
    if (std::ranges::find_if(force_config_hwnds_, [real_fg_win](auto hwnd) {
            return hwnd == real_fg_win;
        }) != force_config_hwnds_.end()) {
        if (last_real_hwnd_ != real_fg_win) {
            last_real_hwnd_ = real_fg_win;
            spdlog::debug("Active window (\"{:#x}\") in launched process window list, forcing specific config", reinterpret_cast<uint64_t>(real_fg_win));
        }
        return target_window_handle_;
    }
    if (last_real_hwnd_ != real_fg_win) {
        last_real_hwnd_ = real_fg_win;
        spdlog::debug("Active window (\"{:#x}\") not in launched process window list, allowing desktop-config", reinterpret_cast<uint64_t>(real_fg_win));
    }
    return real_fg_win;
}
#endif

void SteamTarget::overlayHotkeyWorkaround()
{
    static bool pressed = false;
    if (std::ranges::all_of(overlay_hotkey_,
                            [](const auto& key) {
                                return sf::Keyboard::isKeyPressed(keymap::sfkey[key]);
                            })) {
        spdlog::trace("Detected overlay hotkey(s)");
        pressed = true;
        std::ranges::for_each(overlay_hotkey_, [this](const auto& key) {
#ifdef _WIN32
            PostMessage(target_window_handle_, WM_KEYDOWN, keymap::winkey[key], 0);
#else

#endif
        });
        spdlog::trace("Sending Overlay KeyDown events...");
    }
    else if (pressed) {
        pressed = false;
        std::ranges::for_each(overlay_hotkey_, [this](const auto& key) {
#ifdef _WIN32
            PostMessage(target_window_handle_, WM_KEYUP, keymap::winkey[key], 0);
#else

#endif
        });
        spdlog::trace("Sending Overlay KeyUp events...");
    }
}
