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

#include <utility>

#include <SFML/Window/Event.hpp>

#ifdef _WIN32
#include <Windows.h>
#include <dwmapi.h>
#endif

TargetWindow::TargetWindow(std::function<void()> on_close) : on_close_(std::move(on_close))
{
    window_.create(sf::VideoMode::getDesktopMode(), "GlosSITarget", sf::Style::None);
    window_.setActive(true);

#ifdef _WIN32
    HWND hwnd = window_.getSystemHandle();
    MARGINS margins;
    margins.cxLeftWidth = -1;
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // always on top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#endif

    setClickThrough(true);
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
    window_.clear(sf::Color::Transparent);
    //window_.clear(sf::Color(255,0,0,1));
    window_.display();
}

void TargetWindow::close()
{
    window_.close();
    on_close_();
}
