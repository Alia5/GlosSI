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

#if !defined(WM_DPICHANGED)
#define WM_DPICHANGED 0x02E0
#endif

#endif

TargetWindow::TargetWindow(std::function<void()> on_close, std::vector<std::string> screenshot_hotkey)
    : on_close_(std::move(on_close)),
      screenshot_keys_(std::move(screenshot_hotkey)),
      overlay_(window_, [this]() { close(); })
{
    auto desktop_mode = sf::VideoMode::getDesktopMode();
#ifdef _WIN32
    // For some completely odd reason, the Background becomes black when enabled dpi-awareness and making the window desktop-size.
    // Scaling down by 1px each direction is barely noticeable and works.
    window_.create(sf::VideoMode(desktop_mode.width - 1, desktop_mode.height - 1, 32), "GlosSITarget", sf::Style::None);
    //window_.create(sf::VideoMode(1920, 1080, 32), "GlosSITarget");
#else
    window_.create(desktop_mode, "GlosSITarget", sf::Style::None);
#endif
    window_.setActive(true);

#ifdef _WIN32
    HWND hwnd = window_.getSystemHandle();
    auto dpi = GetWindowDPI(hwnd);
    spdlog::debug("Screen DPI: {}", dpi);

    // transparent windows window...
    auto style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_OVERLAPPED;
    style |= WS_POPUP;
    SetWindowLong(hwnd, GWL_STYLE, style);

    DWM_BLURBEHIND bb{.dwFlags = DWM_BB_ENABLE, .fEnable = true, .hRgnBlur = nullptr};
    DwmEnableBlurBehindWindow(hwnd, &bb);
    MARGINS margins;
    margins.cxLeftWidth = -1;
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    DEVMODE dev_mode = {};
    dev_mode.dmSize = sizeof(DEVMODE);
    dev_mode.dmDriverExtra = 0;

    if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dev_mode) == 0) {
        setFpsLimit(60);
    }
    else {
        setFpsLimit(dev_mode.dmDisplayFrequency);
    }

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = dpi / 96.f;
    ImGui::SFML::UpdateFontTexture();

#else
    setFpsLimit(60);
#endif
}

void TargetWindow::setFpsLimit(unsigned int fps_limit)
{
    window_.setFramerateLimit(fps_limit);
}

void TargetWindow::setClickThrough(bool click_through)
{

#ifdef _WIN32
    HWND hwnd = window_.getSystemHandle();
    if (click_through) {
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_COMPOSITED);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_COMPOSITED);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
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

    window_.clear(sf::Color(0,0,0,0));
    overlay_.update();
    screenShotWorkaround();
    window_.display();
}

void TargetWindow::close()
{
    window_.close();
    Overlay::Shutdown();
    on_close_();
}

Overlay& TargetWindow::getOverlay()
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