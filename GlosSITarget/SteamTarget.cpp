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

SteamTarget::SteamTarget(int argc, char *argv[])
    : window_([this] { run_ = false; }),
      detector_([this](bool overlay_open) { onOverlayChanged(overlay_open); }), target_window_handle_(window_.getSystemHandle())
{
}

int SteamTarget::run()
{
    run_ = true;
    window_.setFpsLimit(90);
    while (run_) {
        detector_.update();
        window_.update();
        overlayHotkeyWorkaround();
    }
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

    //MH_DisableHook(&GetForegroundWindow); // TODO: when GetForegroundWindow hooked, unhook!
    // store last focused window for later restore
    last_foreground_window_ = GetForegroundWindow();
    const DWORD fg_thread = GetWindowThreadProcessId(last_foreground_window_, nullptr);
    //MH_EnableHook(&GetForegroundWindow); // TODO: when GetForegroundWindow hooked, re-hook!

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
    return key.GetStringValue(L"SteamPath");
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
    return std::to_wstring(key.GetDwordValue(L"ActiveUser"));
#else
    return L""; // TODO
#endif
}

std::vector<std::string> SteamTarget::getOverlayHotkey()
{
    const auto steam_path = getSteamPath();
    const auto user_id = getSteamUserId();

    const auto config_path = steam_path + std::wstring(user_data_path_) + user_id + std::wstring(config_file_name_);
    std::ifstream config_file(config_path);
    // TODO: check if file exists
    auto root = tyti::vdf::read(config_file);

    auto children = root.childs["system"];
    auto hotkeys = children->attribs.at("InGameOverlayShortcutKey");

    // has anyone more than 4 keys to open overlay?!
    std::smatch m;
    if (!std::regex_match(hotkeys, m, std::regex(R"((\w*)\s*(\w*)\s*(\w*)\s*(\w*))"))) {
        return {"Shift", "KEY_TAB"};
    }

    std::vector<std::string> res;
    for (auto i = 1; i < m.size(); i++) {
        const auto s = std::string(m[i]);
        if (!s.empty()) {
            res.push_back(s);
        }
    }
    spdlog::info("Detected Overlay hotkeys: {}", std::accumulate(
                                                     res.begin() + 1, res.end(), res[0],
                                                     [](auto acc, const auto curr) { return acc += "+" + curr; }));
    return res;
}

void SteamTarget::overlayHotkeyWorkaround()
{
    static bool pressed = false;
    if (std::all_of(overlay_hotkey_.begin(), overlay_hotkey_.end(),
                    [](const auto &key) {
                        return sf::Keyboard::isKeyPressed(keymap::sfkey[key]);
                    })) {
        pressed = true;
        std::for_each(
            overlay_hotkey_.begin(), overlay_hotkey_.end(), [this](const auto &key) {
#ifdef _WIN32
                PostMessage(target_window_handle_, WM_KEYDOWN, keymap::winkey[key], 0);
#else

#endif
            });
        spdlog::debug("Sending Overlay KeyDown events...");
    } else if (pressed) {
        pressed = false;
        std::for_each(
            overlay_hotkey_.begin(), overlay_hotkey_.end(), [this](const auto &key) {
#ifdef _WIN32
                PostMessage(target_window_handle_, WM_KEYUP, keymap::winkey[key], 0);
#else

#endif
            });
        spdlog::debug("Sending Overlay KeyUp events...");
    }
}
