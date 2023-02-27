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
#include "SteamTarget.h"

#include "../common/Settings.h"
#include "steam_sf_keymap.h"

#include <SFML/Window/Keyboard.hpp>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include "UWPOverlayEnabler.h"
#include <tray.hpp>
#endif

#include <CEFInject.h>

#include "CommonHttpEndpoints.h"

SteamTarget::SteamTarget()
    : window_(
          [this] { run_ = false; },
          [this] { toggleGlossiOverlay(); },
          util::steam::getScreenshotHotkey(steam_path_, steam_user_id_),
          [this]() {
              target_window_handle_ = window_.getSystemHandle();
              overlay_ = window_.getOverlay();
          }),
      overlay_(window_.getOverlay()),
      detector_([this](bool overlay_open) { onOverlayChanged(overlay_open); }),
      launcher_(force_config_hwnds_, [this] {
          delayed_shutdown_ = true;
          delay_shutdown_clock_.restart();
      }),
      server_([this] { run_ = false; })
{
    target_window_handle_ = window_.getSystemHandle();
}

int SteamTarget::run()
{
    run_ = true;
    auto closeBPM = false;
    auto closeBPMTimer = sf::Clock{};
    if (!SteamOverlayDetector::IsSteamInjected()) {
        if (Settings::common.allowStandAlone) {
            spdlog::warn("GlosSI not launched via Steam.\nEnabling EXPERIMENTAL global controller and overlay...");
            if (Settings::common.standaloneModeGameId == L"") {
                spdlog::error("No game id set for standalone mode. Controller will use desktop-config!");
            }
    auto steam_tweaks = CEFInject::SteamTweaks();
    steam_tweaks.setAutoInject(true);

    CHTE::addEndpoints();

    server_.run();


    if (!overlay_.expired())
        overlay_.lock()->setEnabled(false);

    std::vector<std::function<void()>> end_frame_callbacks;

    if (!CEFInject::CEFDebugAvailable()) {
        auto overlay_id = std::make_shared<int>(-1);
        *overlay_id = Overlay::AddOverlayElem(
            [this, overlay_id, &end_frame_callbacks](bool window_has_focus, ImGuiID dockspace_id) {
                can_fully_initialize_ = false;
                ImGui::Begin("GlosSI - CEF Debug not available");
                ImGui::Text("GlosSI makes use of Steam CEF Debugging for some functionality and plugins.");
                ImGui::Text("GlosSI might not work fully without it.");

                if (ImGui::Button("Ignore and continue")) {
                    can_fully_initialize_ = true;
                    cef_tweaks_enabled_ = false;
                    if (*overlay_id != -1) {
                        end_frame_callbacks.emplace_back([this, overlay_id] {
                            Overlay::RemoveOverlayElem(*overlay_id);
                        });
                    }
                }

                if (ImGui::Button("Enable and restart Steam")) {

                    std::ofstream{steam_path_ / ".cef-enable-remote-debugging"};
                    system("taskkill.exe /im steam.exe /f");
                    Sleep(200);
                    launcher_.launchApp((steam_path_ / "Steam.exe").wstring());

                    run_ = false;
                }
                ImGui::Text("GlosSI will close upon restarting Steam");

                ImGui::End();
            },
            true);
        can_fully_initialize_ = false;
        cef_tweaks_enabled_ = false;
    }

    if (!SteamOverlayDetector::IsSteamInjected() && Settings::common.allowGlobalMode && Settings::common.globalModeGameId == L"") {
        auto overlay_id = std::make_shared<int>(-1);
        *overlay_id = Overlay::AddOverlayElem(
            [this, overlay_id, &end_frame_callbacks](bool window_has_focus, ImGuiID dockspace_id) {
                can_fully_initialize_ = false;
                ImGui::Begin("Global mode", nullptr, ImGuiWindowFlags_NoSavedSettings);
                ImGui::Text("You are running GlosSI in (experimental) global mode (=outside of Steam)");
                ImGui::Text("but global mode doesn't appear to be setup properly.");
                ImGui::Text("");
                ImGui::Text("Please open GlosSI-Config first and setup global mode");
                ImGui::Text("");
                ImGui::Text("Application will exit on confirm");
                if (ImGui::Button("OK")) {
                    can_fully_initialize_ = true;
                    if (*overlay_id != -1) {
                        end_frame_callbacks.emplace_back([this, overlay_id] {
                            Overlay::RemoveOverlayElem(*overlay_id);
                            run_ = false;
                        });
                    }
                }
                ImGui::End();
            },
            true);
        can_fully_initialize_ = false;
    }

    if (!util::steam::getXBCRebindingEnabled(steam_path_, steam_user_id_)) {
        auto overlay_id = std::make_shared<int>(-1);
        *overlay_id = Overlay::AddOverlayElem(
            [this, overlay_id, &end_frame_callbacks](bool window_has_focus, ImGuiID dockspace_id) {
                can_fully_initialize_ = false;
                ImGui::Begin("XBox Controller configuration support Disabled", nullptr, ImGuiWindowFlags_NoSavedSettings);
                ImGui::TextColored({1.f, 0.8f, 0.f, 1.f}, "XBox Controller configuration support is disabled in Steam. Please enable it in Steam Settings.");
                if (ImGui::Button("OK")) {
                    can_fully_initialize_ = true;
                    if (*overlay_id != -1) {
                        end_frame_callbacks.emplace_back([this, overlay_id] {
                            Overlay::RemoveOverlayElem(*overlay_id);
                        });
                    }
                }
                ImGui::End();
            },
            true);
        can_fully_initialize_ = false;
    }
    
    const auto tray = createTrayMenu();
    
    bool delayed_full_init_1_frame = false;
    sf::Clock frame_time_clock;

    while (run_) {
        if (!fully_initialized_ && can_fully_initialize_ && delayed_full_init_1_frame) {
            init_FuckingRenameMe();
        }
        else if (!fully_initialized_ && can_fully_initialize_) {
            delayed_full_init_1_frame = true;
        }
        else {
            delayed_full_init_1_frame = false;
        }
        detector_.update();
        overlayHotkeyWorkaround();
        window_.update();

        if (cef_tweaks_enabled_ && fully_initialized_) {
            steam_tweaks_.update(frame_time_clock.getElapsedTime().asSeconds());
        }

        // Wait on shutdown; User might get confused if window closes to fast if anything with launchApp get's borked.
        if (delayed_shutdown_) {
            if (delay_shutdown_clock_.getElapsedTime().asSeconds() >= 3) {
                run_ = false;
            }
        }
        else {
            if (fully_initialized_) {
                launcher_.update();
            }
        }
        for (auto& efc : end_frame_callbacks) {
            efc();
        }
        end_frame_callbacks.clear();
        frame_time_clock.restart();
    }
    tray->exit();

    server_.stop();
    if (fully_initialized_) {
#ifdef _WIN32
        input_redirector_.stop();
        hidhide_.disableHidHide();
#endif
        launcher_.close();
        if (cef_tweaks_enabled_) {
            steam_tweaks_.uninstallTweaks();
        }
    }

    return 0;
}

void SteamTarget::onOverlayChanged(bool overlay_open)
{
    if (overlay_open) {
        focusWindow(target_window_handle_);
        window_.setClickThrough(!overlay_open);
        if (!Settings::window.windowMode && Settings::window.opaqueSteamOverlay) {
            window_.setTransparent(false);
        }
    }
    else {
        if (!(overlay_.expired() ? false : overlay_.lock()->isEnabled())) {
            window_.setClickThrough(!overlay_open);
            focusWindow(last_foreground_window_);
            if (!Settings::window.windowMode && Settings::window.opaqueSteamOverlay) {
                window_.setTransparent(true);
            }
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
    if (Settings::window.disableGlosSIOverlay) {
        return;
    }
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
    if (reinterpret_cast<uint64_t>(hndl) == 0) {
        return;
    }
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

    // try to forcefully set foreground window at least a few times
    sf::Clock clock;
    while (!SetForegroundWindow(hndl) && clock.getElapsedTime().asMilliseconds() < 20) {
        SetActiveWindow(hndl);
        Sleep(1);
    }

#endif
}
void SteamTarget::init_FuckingRenameMe()
{
    if (!SteamOverlayDetector::IsSteamInjected()) {
        if (Settings::common.allowGlobalMode) {
            spdlog::warn("GlosSI not launched via Steam.\nEnabling EXPERIMENTAL global controller and overlay...");
            if (Settings::common.globalModeGameId == L"") {
                spdlog::error("No game id set for global mode. Controller will use desktop-config!");
            }

            SetEnvironmentVariable(L"SteamAppId", L"0");
            SetEnvironmentVariable(L"SteamClientLaunch", L"0");
            SetEnvironmentVariable(L"SteamEnv", L"1");
            SetEnvironmentVariable(L"SteamPath", steam_path_.wstring().c_str());
            SetEnvironmentVariable(L"SteamTenfoot", Settings::common.globalModeUseGamepadUI ? L"1" : L"0");
            // SetEnvironmentVariable(L"SteamTenfootHybrid", L"1");
            SetEnvironmentVariable(L"SteamGamepadUI", Settings::common.globalModeUseGamepadUI ? L"1" : L"0");
            SetEnvironmentVariable(L"SteamGameId", Settings::common.globalModeGameId.c_str());
            SetEnvironmentVariable(L"SteamOverlayGameId", Settings::common.globalModeGameId.c_str());
            SetEnvironmentVariable(L"EnableConfiguratorSupport", L"15");
            SetEnvironmentVariable(L"SteamStreamingForceWindowedD3D9", L"1");

            if (Settings::common.globalModeUseGamepadUI) {
                system("start steam://open/bigpicture");
                auto steamwindow = FindWindow(L"Steam Big Picture Mode", nullptr);
                auto timer = sf::Clock{};
                while (!steamwindow && timer.getElapsedTime().asSeconds() < 2) {
                    steamwindow = FindWindow(L"Steam Big Picture Mode", nullptr);
                    Sleep(50);
                }

                if (cef_tweaks_enabled_) {
                    steam_tweaks_.setAutoInject(true);
                    steam_tweaks_.update(999);
                }

                Sleep(6000); // DIRTY HACK to wait until BPM (GamepadUI) is initialized
                // TODO: find way to force BPM even if BPM is not active
                LoadLibrary((steam_path_ / "GameOverlayRenderer64.dll").wstring().c_str());

                // Overlay switches back to desktop one, once BPM is closed... Disable closing BPM for now.
                // TODO: find way to force BPM even if BPM is not active
                // closeBPM = true;
                // closeBPMTimer.restart();
            }
            else {
                LoadLibrary((steam_path_ / "GameOverlayRenderer64.dll").wstring().c_str());
            }

            window_.setClickThrough(true);
            steam_overlay_present_ = true;
        }
        else {
            spdlog::warn("Steam-overlay not detected and global mode disabled. Showing GlosSI-overlay!\n\
Application will not function!");
            window_.setClickThrough(false);
            if (!overlay_.expired())
                overlay_.lock()->setEnabled(true);
            steam_overlay_present_ = false;
        }
    }
    else {
        spdlog::info("Steam-overlay detected.");
        spdlog::warn("Double press Steam- overlay key(s)/Controller button to show GlosSI-overlay"); // Just to color output and really get users attention
        window_.setClickThrough(true);
        steam_overlay_present_ = true;
    }

#ifdef WIN32
    if (!Settings::common.disable_watchdog) {
        wchar_t buff[MAX_PATH];
        GetModuleFileName(GetModuleHandle(NULL), buff, MAX_PATH);
        std::wstring watchDogPath(buff);
        watchDogPath = watchDogPath.substr(0, 1 + watchDogPath.find_last_of(L'\\')) + L"GlosSIWatchdog.dll";

        DllInjector::injectDllInto(watchDogPath, L"explorer.exe");
    }

    if (Settings::common.no_uwp_overlay) {
        UWPOverlayEnabler::AddUwpOverlayOvWidget();
    }
    else {
        UWPOverlayEnabler::EnableUwpOverlay();
    }

    hidhide_.hideDevices(steam_path_);
    input_redirector_.run();
#endif
    if (Settings::launch.launch) {
        launcher_.launchApp(Settings::launch.launchPath, Settings::launch.launchAppArgs);
    }
    keepControllerConfig(true);

    if (cef_tweaks_enabled_) {
        steam_tweaks_.setAutoInject(true);
    }

    fully_initialized_ = true;
}

/*
 * The "magic" that keeps a controller-config forced (without hooking into Steam)
 *
 * Hook into own process and detour "GetForegroundWindow"
 * Detour function always returns HWND of own application window
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

std::unique_ptr<Tray::Tray> SteamTarget::createTrayMenu()
{
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
    auto tray = std::make_unique<Tray::Tray>("GlosSITarget", icon);
#else
    auto tray = std::make_unique<Tray::Tray>("GlosSITarget", "ico.png");
#endif

    tray->addEntry(Tray::Button{
        "Quit", [this, &tray]() {
            run_ = false;
        }});
    return tray;
}

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
