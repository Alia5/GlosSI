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
#pragma once
#include "Overlay.h"

#include <functional>

#include <SFML/Graphics/RenderWindow.hpp>

// Redfine window handle, should impl. change
#ifdef _WIN32
#include <Windows.h>
using WindowHandle = HWND;
#else
using WindowHandle = int; // ???
#endif

class TargetWindow {
  public:
    explicit TargetWindow(
        std::function<void()> on_close = []() {},
        std::function<void()> toggle_overlay_state = []() {},
        std::vector<std::string> screenshot_hotkey = {"KEY_F12"},
        std::function<void()> on_window_changed = []() {}
    );

    void setFpsLimit(unsigned int fps_limit);
    void setClickThrough(bool click_through);
    void update();
    void close();

    std::shared_ptr<Overlay> getOverlay() const;

    /*
     * Run once per frame
     * - detects steam configured screenshot hotkey
     * - takes actual screenshot
     * - renders it to window
     * - simulates screenshot keys
     * - Wait a few millis...
     * (- steam takes screenshot)
     * - return to normal
     * 
     */
    void screenShotWorkaround();

    WindowHandle getSystemHandle() const;

#ifdef _WIN32
    static WORD GetWindowDPI(HWND hWnd);
#endif

  private:
    const std::function<void()> on_close_;
    const std::function<void()> toggle_overlay_state_;
    sf::RenderWindow window_;
    std::vector<std::string> screenshot_keys_;
    const std::function<void()> on_window_changed_;

    sf::VideoMode old_desktop_mode_;
    sf::Clock check_resolution_clock_;
    static constexpr int RES_CHECK_SECONDS = 1;


    std::shared_ptr<Overlay> overlay_;

    void createWindow();

    bool toggle_window_mode_after_frame_ = false;
};
