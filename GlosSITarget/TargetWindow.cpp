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

#include <iostream>
#include <utility>

#include <SFML/Window/Event.hpp>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <dwmapi.h>
#endif

static const bool DEV_MODE = false;

TargetWindow::TargetWindow(std::function<void()> on_close, std::vector<std::string> screenshot_hotkey)
    : on_close_(std::move(on_close)), screenshot_keys_(std::move(screenshot_hotkey))
{
    if (DEV_MODE) {
        window_.create(sf::VideoMode{1920, 1080}, "GlosSITarget", sf::Style::Default);
    }
    else {
        window_.create(sf::VideoMode::getDesktopMode(), "GlosSITarget", sf::Style::None);
    }
    window_.setActive(true);

#ifdef _WIN32
    HWND hwnd = window_.getSystemHandle();
    MARGINS margins;
    margins.cxLeftWidth = -1;
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    if (!DEV_MODE) {
        // always on top
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#endif

    if (!DEV_MODE) {
        setClickThrough(true);
    }
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
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED);
    }
#endif
}

void TargetWindow::update()
{
    sf::Event event{};
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
            on_close_();
        }
    }

    if (DEV_MODE) {
        window_.clear(sf::Color(0, 0, 0, 128));
    }
    else {
        window_.clear(sf::Color::Transparent);
    }

    screenShotWorkaround();
    window_.display();
}

void TargetWindow::close()
{
    window_.close();
    on_close_();
}

void TargetWindow::screenShotWorkaround()
{
#ifdef _WIN32
    if (std::ranges::all_of(screenshot_keys_,
                            [](const auto &key) {
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
        std::ranges::for_each(screenshot_keys_, [this](const auto &key) {
            PostMessage(window_.getSystemHandle(), WM_KEYDOWN, keymap::winkey[key], 0);
        });
        std::ranges::for_each(screenshot_keys_, [this](const auto &key) {
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
