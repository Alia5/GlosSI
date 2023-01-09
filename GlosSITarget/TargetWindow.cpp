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
#include "TargetWindow.h"

#include "steam_sf_keymap.h"

#include <utility>

#include <SFML/Window/Event.hpp>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <SFML/Graphics.hpp>
#include <VersionHelpers.h>
#include <Windows.h>
#include <dwmapi.h>

#include "ProcessPriority.h"

#include "Settings.h"

#if !defined(WM_DPICHANGED)
#define WM_DPICHANGED 0x02E0
#endif

#endif

TargetWindow::TargetWindow(
    std::function<void()> on_close,
    std::function<void()> toggle_overlay_state,
    std::vector<std::string> screenshot_hotkey,
    std::function<void()> on_window_changed)
    : on_close_(std::move(on_close)),
      toggle_overlay_state_(std::move(toggle_overlay_state)),
      screenshot_keys_(std::move(screenshot_hotkey)),
      on_window_changed_(std::move(on_window_changed))
{
    createWindow();

    Overlay::AddOverlayElem([this](bool window_has_focus, ImGuiID dockspace_id) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::Begin("Window");
        if (ImGui::Checkbox("Window mode", &Settings::window.windowMode)) {
            toggle_window_mode_after_frame_ = true;
        }
#ifdef _WIN32
        if (ImGui::Checkbox("Hide from Alt+Tab", &Settings::window.hideAltTab)) {
            toggle_hidealttab_after_frame_ = true;
        }
#endif
        ImGui::Text("Max. FPS");
        ImGui::SameLine();
        int max_fps_copy = Settings::window.maxFps;
        ImGui::InputInt("##max_fps", &max_fps_copy, 20, 20);
        ImGui::Text("Values smaller than 15 set the limit to the screen refresh rate.");
        if (max_fps_copy != Settings::window.maxFps) {
            Settings::window.maxFps = max_fps_copy; 
            if (Settings::window.maxFps > 240) {
                Settings::window.maxFps = 240;
            }
            if (Settings::window.maxFps < 15 && Settings::window.maxFps > 0) {
                Settings::window.maxFps = 0;
                setFpsLimit(screen_refresh_rate_);
            } else {
                setFpsLimit(Settings::window.maxFps);
            }
        }

        ImGui::Spacing();

        ImGui::Text("Overlay scale");
        ImGui::SameLine();
        float scale_copy = Settings::window.scale;
        ImGui::DragFloat("##UISCale", &scale_copy, 0.1f, 0.0f, 6.f);
        ImGui::Text("Values smaller than 0.3 reset to 1");
        if (scale_copy > Settings::window.scale + 0.01f || scale_copy < Settings::window.scale - 0.01f) {
            Settings::window.scale = scale_copy;
            if (Settings::window.scale < 0.3f) {
                spdlog::trace("Scale to small! Scaling overlay to 1");
                Settings::window.scale = 0.0f;
                ImGuiIO& io = ImGui::GetIO();
                io.FontGlobalScale = 1;
                ImGui::SFML::UpdateFontTexture();
            } else {
                spdlog::trace("Scaling overlay: {}", Settings::window.scale);
                ImGuiIO& io = ImGui::GetIO();
                io.FontGlobalScale = Settings::window.scale;
                ImGui::SFML::UpdateFontTexture();   
            }
        }

        ImGui::End();
    });

    ProcessPriority::init();
}

void TargetWindow::setFpsLimit(unsigned int fps_limit)
{
    spdlog::trace("Limiting FPS to {}", fps_limit);
    window_.setFramerateLimit(fps_limit);
}

void TargetWindow::setClickThrough(bool click_through)
{
    if (Settings::window.windowMode) {
        return;
    }
#ifdef _WIN32
    HWND hwnd = window_.getSystemHandle();

    // hiding GlosSI from Alt-Tab list
    // https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
    if (Settings::window.hideAltTab) {
        toggle_hidealttab_after_frame_ = false;

        if (click_through) {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_COMPOSITED | WS_EX_TOOLWINDOW);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_COMPOSITED | WS_EX_TOOLWINDOW);
            SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    else {
        if (click_through) {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_COMPOSITED);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_COMPOSITED);
            SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
#endif
}

void TargetWindow::update()
{
    sf::Event event{};
    while (window_.pollEvent(event)) {
        Overlay::ProcessEvent(event);
        if (event.type == sf::Event::Closed) {
            close();
            return;
        }
    }
    // windows clear always handled in overlay. => non fully transparent
    window_.clear(sf::Color(0, 0, 0, 0));
    screenShotWorkaround();
    overlay_->update();
    window_.display();
#ifdef _WIN32
    if (toggle_hidealttab_after_frame_) {
        toggle_hidealttab_after_frame_ = false;
    }
#endif
    if (toggle_window_mode_after_frame_) {
        createWindow();
    }

    // As SFML screws us out of most windows-events, just poll resolution every once in a while
    // If changed, recreate window.
    // Fixes Blackscreen issues when user does funky stuff and still uses GlosSI in non windowed mod...
    // (WHY?!)
    if (check_resolution_clock_.getElapsedTime().asSeconds() > RES_CHECK_SECONDS) {
        if (sf::VideoMode::getDesktopMode().width != old_desktop_mode_.width) {
            createWindow();
        }
        check_resolution_clock_.restart();
    }
}

void TargetWindow::close()
{
    window_.close();
    Overlay::Shutdown();
    on_close_();
}

std::shared_ptr<Overlay> TargetWindow::getOverlay() const
{
    return overlay_;
}

void TargetWindow::screenShotWorkaround()
{
#ifdef _WIN32
    if (std::ranges::all_of(screenshot_keys_,
                            [](const auto& key) {
                                return sf::Keyboard::isKeyPressed(keymap::sfkey[key]);
                            })) {
        spdlog::debug("Detected screenshot hotkey(s); Taking screenshot");

        // stolen from: https://en.sfml-dev.org/forums/index.php?topic=14323.15
        // no time to do this shit.
        HDC hScreenDC = GetDC(nullptr);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
        int width = GetDeviceCaps(hScreenDC, HORZRES);
        int height = GetDeviceCaps(hScreenDC, VERTRES);
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
        BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
        hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));
        BITMAP bm;
        GetObject(hBitmap, sizeof(bm), &bm);
        BITMAPINFO bmpInfo;
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = bm.bmWidth;
        bmpInfo.bmiHeader.biHeight = -bm.bmHeight;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = 0;
        bmpInfo.bmiHeader.biClrImportant = 0;
        std::vector<COLORREF> pixel;
        pixel.resize(bm.bmWidth * bm.bmHeight);
        sf::Image captureImage;
        captureImage.create(bm.bmWidth, bm.bmHeight, sf::Color::Black);
        GetDIBits(hMemoryDC, hBitmap, 0, bm.bmHeight, pixel.data(), &bmpInfo, DIB_RGB_COLORS);
        unsigned int j = 0;
        for (unsigned int y = 0; y < bm.bmHeight; ++y) {
            for (unsigned int x = 0; x < bm.bmWidth; ++x) {
                const COLORREF this_color = pixel[j++];
                captureImage.setPixel(x, y, sf::Color(GetBValue(this_color), GetGValue(this_color), GetRValue(this_color)));
            }
        }
        ReleaseDC(NULL, hScreenDC);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        DeleteDC(hScreenDC);

        sf::Sprite sprite;
        sf::Texture texture;
        texture.loadFromImage(captureImage);
        sprite.setTexture(texture);

        spdlog::debug("Sending screenshot key events and rendering screen...");
        std::ranges::for_each(screenshot_keys_, [this](const auto& key) {
            PostMessage(window_.getSystemHandle(), WM_KEYDOWN, keymap::winkey[key], 0);
        });
        std::ranges::for_each(screenshot_keys_, [this](const auto& key) {
            PostMessage(window_.getSystemHandle(), WM_KEYUP, keymap::winkey[key], 0);
        });
        //actually run event loop, so steam gets notified about keys.
        sf::Event event{};
        while (window_.pollEvent(event)) {
        }
        // steam takes screenshot on next frame, so render our screenshot and dipslay...
        window_.clear(sf::Color::Black);
        window_.draw(sprite);
        window_.display();
        // finally, draw another transparent frame.
        window_.clear(sf::Color::Transparent);
    }
#endif
}

WindowHandle TargetWindow::getSystemHandle() const
{
    return window_.getSystemHandle();
}

#ifdef _WIN32
// stolen from: https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-3/
typedef HRESULT(WINAPI* PGetDpiForMonitor)(HMONITOR hmonitor, int dpiType, UINT* dpiX, UINT* dpiY);
WORD TargetWindow::GetWindowDPI(HWND hWnd)
{
    // Try to get the DPI setting for the monitor where the given window is located.
    // This API is Windows 8.1+.
    HMODULE hShcore = LoadLibraryW(L"shcore");
    if (hShcore) {
        PGetDpiForMonitor pGetDpiForMonitor =
            reinterpret_cast<PGetDpiForMonitor>(GetProcAddress(hShcore, "GetDpiForMonitor"));
        if (pGetDpiForMonitor) {
            HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
            UINT uiDpiX;
            UINT uiDpiY;
            HRESULT hr = pGetDpiForMonitor(hMonitor, 0, &uiDpiX, &uiDpiY);
            if (SUCCEEDED(hr)) {
                return static_cast<WORD>(uiDpiX);
            }
        }
    }

    // We couldn't get the window's DPI above, so get the DPI of the primary monitor
    // using an API that is available in all Windows versions.
    HDC hScreenDC = GetDC(0);
    int iDpiX = GetDeviceCaps(hScreenDC, LOGPIXELSX);
    ReleaseDC(0, hScreenDC);

    return static_cast<WORD>(iDpiX);
}
#endif

void TargetWindow::createWindow()
{
    toggle_window_mode_after_frame_ = false;

    auto desktop_mode = sf::VideoMode::getDesktopMode();
    spdlog::info("Detected resolution: {}x{}", desktop_mode.width, desktop_mode.height);
    old_desktop_mode_ = desktop_mode;
    if (Settings::window.windowMode) {
        spdlog::info("Creating Overlay window...");
        window_.create(sf::VideoMode(desktop_mode.width * 0.75, desktop_mode.height * 0.75, 32), "GlosSITarget");
    }
    else {
#ifdef _WIN32
        // For some completely odd reason, the Background becomes black when enabled dpi-awareness and making the window desktop-size.
        // Scaling down by 1px each direction is barely noticeable and works.

        // Due to some other issue, the (Steam) overlay might get blurred when doing this
        // as a workaround, start in full size, and scale down later...
        spdlog::info("Creating Overlay window (Borderless Fullscreen)...");
        window_.create(sf::VideoMode(desktop_mode.width, desktop_mode.height, 32), "GlosSITarget", sf::Style::None);

        // get size of all monitors combined
        const auto screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        const auto screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        spdlog::debug("Full screen size: {}x{}", screenWidth, screenHeight);

        spdlog::debug("Primary monitor size: {}x{}", desktop_mode.width, desktop_mode.height);

#else
        window_.create(desktop_mode, "GlosSITarget", sf::Style::None);
#endif
    }
    window_.setActive(true);
    spdlog::debug("Window position: {}x{}", window_.getPosition().x, window_.getPosition().y);

    if (!Settings::window.windowMode) {
        spdlog::info("Resizing window to 1px smaller than fullscreen...");
        window_.setSize(sf::Vector2u(desktop_mode.width - 1, desktop_mode.height - 1));
    }

#ifdef _WIN32
    HWND hwnd = window_.getSystemHandle();
    auto dpi = GetWindowDPI(hwnd);
    spdlog::debug("Screen DPI: {}", dpi);

    //if (windowed_) {
    //    DWM_BLURBEHIND bb{.dwFlags = DWM_BB_ENABLE, .fEnable = true, .hRgnBlur = nullptr};
    //    DwmEnableBlurBehindWindow(hwnd, &bb);
    //} // semi-transparent in window mode, but deprecated api
    // On Linux the window will (should) automagically be semi-transparent

    // transparent windows window...
    auto style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_OVERLAPPED;
    style |= WS_POPUP;
    SetWindowLong(hwnd, GWL_STYLE, style);
    

    MARGINS margins;
    margins.cxLeftWidth = -1;
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    DEVMODE dev_mode = {};
    dev_mode.dmSize = sizeof(DEVMODE);
    dev_mode.dmDriverExtra = 0;

    if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dev_mode) == 0) {
        setFpsLimit(60);
        spdlog::warn("Couldn't detect screen refresh rate; Limiting overlay to 60");
        screen_refresh_rate_ = 60;
    }
    else {
        setFpsLimit(dev_mode.dmDisplayFrequency);
        screen_refresh_rate_ = dev_mode.dmDisplayFrequency;
    }

    overlay_ = std::make_shared<Overlay>(
        window_, [this]() { close(); }, toggle_overlay_state_, Settings::window.windowMode);

    spdlog::debug("auto screen sCale: {}", dpi/96.f);
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = dpi / 96.f;
    ImGui::SFML::UpdateFontTexture();

#else
    setFpsLimit(60);
#endif

    if (Settings::window.maxFps > 0) {
        spdlog::debug("Config file fps limit seems sane...");
        setFpsLimit(Settings::window.maxFps);
    }
    if (Settings::window.scale > 0.3f) { // Now that's just getting ridicoulus
        spdlog::debug("setting screen scale by config: {}", Settings::window.scale);
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = Settings::window.scale;
        ImGui::SFML::UpdateFontTexture();
    }
    else {
        spdlog::debug("Not applying too low screen scale setting");
    }

    // window_.setSize({desktop_mode.width - 1, desktop_mode.height - 1 });

    on_window_changed_();

#ifdef _WIN32
    if (Settings::window.disableOverlay) {
        setFpsLimit(1);
        ShowWindow(hwnd, SW_HIDE);
    }
#endif
}
