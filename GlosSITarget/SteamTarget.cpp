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
#include "SteamTarget.h"

#include "steam_sf_keymap.h"

#include <SFML/Window/Keyboard.hpp>
#include <WinReg/WinReg.hpp>
#include <numeric>
#include <regex>
#include <spdlog/spdlog.h>
#include <vdf_parser.hpp>

#include <subhook.h>

#ifdef _WIN32
subhook::Hook getFgWinHook;
static HWND target_hwnd = nullptr;

HWND keepForegroundWindow()
{
    return target_hwnd;
}

#endif

SteamTarget::SteamTarget(int argc, char *argv[])
    : window_([this] { run_ = false; }, getScreenshotHotkey()),
      detector_([this](bool overlay_open) { onOverlayChanged(overlay_open); }), target_window_handle_(window_.getSystemHandle())
{
#ifdef _WIN32
    target_hwnd = target_window_handle_;
#endif
}

int SteamTarget::run()
{
    run_ = true;
    keepControllerConfig(true);
    input_redirector_.run();
    while (run_) {
        detector_.update();
        window_.update();
        overlayHotkeyWorkaround();
    }
    input_redirector_.stop();
    return 1;
}

void SteamTarget::onOverlayChanged(bool overlay_open)
{
    window_.setClickThrough(!overlay_open);
    if (overlay_open) {
        focusWindow(target_window_handle_);
    }
    else {
        focusWindow(last_foreground_window_);
    }
}

void SteamTarget::focusWindow(WindowHandle hndl)
{
#ifdef _WIN32

    if (hndl == target_window_handle_) {
        spdlog::info("Bring own window to foreground");
    }
    else {
        spdlog::info("Bring window \"{:#x}\" to foreground", reinterpret_cast<uint64_t>(hndl));
    }

    keepControllerConfig(false); // unhook GetForegroundWindow
    last_foreground_window_ = GetForegroundWindow();
    const DWORD fg_thread = GetWindowThreadProcessId(last_foreground_window_, nullptr);
    keepControllerConfig(true); // re-hook GetForegroundWindow

    // lot's of ways actually bringing our window to foreground...
    const DWORD current_thread = GetCurrentThreadId();
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

std::wstring SteamTarget::getSteamPath()
{
#ifdef _WIN32
    // TODO: check if keys/value exist
    // steam should always be open and have written reg values...
    winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam"};
    const auto res = key.GetStringValue(L"SteamPath");
    spdlog::info(L"Detected Steam Path: {}", res);
    return res;
#else
    return L""; // TODO
#endif
}

std::wstring SteamTarget::getSteamUserId()
{
#ifdef _WIN32
    // TODO: check if keys/value exist
    // steam should always be open and have written reg values...
    winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess"};
    const auto res = std::to_wstring(key.GetDwordValue(L"ActiveUser"));
    spdlog::info(L"Detected Steam UserId: {}", res);
    return res;
#else
    return L""; // TODO
#endif
}

std::vector<std::string> SteamTarget::getOverlayHotkey()
{
    const auto config_path = steam_path_ + std::wstring(user_data_path_) + steam_user_id_ + std::wstring(config_file_name_);
    std::ifstream config_file(config_path);
    // TODO: check if file exists
    auto root = tyti::vdf::read(config_file);

    auto children = root.childs["system"];
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
    const auto config_path = steam_path_ + std::wstring(user_data_path_) + steam_user_id_ + std::wstring(config_file_name_);
    std::ifstream config_file(config_path);
    // TODO: check if file exists
    auto root = tyti::vdf::read(config_file);

    auto children = root.childs["system"];
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

void SteamTarget::keepControllerConfig(bool keep)
{
#ifdef _WIN32
    if (keep && !getFgWinHook.IsInstalled()) {
        spdlog::debug("Hooking GetForegroudnWindow (in own process)");
        getFgWinHook.Install(&GetForegroundWindow, &keepForegroundWindow, subhook::HookFlags::HookFlag64BitOffset);
        if (!getFgWinHook.IsInstalled()) {
            spdlog::error("Couldn't install GetForegroundWindow hook!");
        }
    }
    else if (!keep && getFgWinHook.IsInstalled()) {
        spdlog::debug("Un-Hooking GetForegroudnWindow (in own process)");
        getFgWinHook.Remove();
        if (getFgWinHook.IsInstalled()) {
            spdlog::error("Couldn't un-install GetForegroundWindow hook!");
        }
    }
#endif
}

void SteamTarget::overlayHotkeyWorkaround()
{
    static bool pressed = false;
    if (std::ranges::all_of(overlay_hotkey_,
                            [](const auto &key) {
                                return sf::Keyboard::isKeyPressed(keymap::sfkey[key]);
                            })) {
        spdlog::debug("Detected overlay hotkey(s)");
        pressed = true;
        std::ranges::for_each(overlay_hotkey_, [this](const auto &key) {
#ifdef _WIN32
            PostMessage(target_window_handle_, WM_KEYDOWN, keymap::winkey[key], 0);
#else

#endif
        });
        spdlog::debug("Sending Overlay KeyDown events...");
    }
    else if (pressed) {
        pressed = false;
        std::ranges::for_each(overlay_hotkey_, [this](const auto &key) {
#ifdef _WIN32
            PostMessage(target_window_handle_, WM_KEYUP, keymap::winkey[key], 0);
#else

#endif
        });
        spdlog::debug("Sending Overlay KeyUp events...");
    }
}
