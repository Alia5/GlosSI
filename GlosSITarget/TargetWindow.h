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
#pragma once
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
        std::function<void()> on_close = []() {}, std::vector<std::string> screenshot_hotkey = {"KEY_F12"});

    void setFpsLimit(unsigned int fps_limit);
    void setClickThrough(bool click_through);
    void update();
    void close();

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

  private:
    const std::function<void()> on_close_;
    sf::RenderWindow window_;
    std::vector<std::string> screenshot_keys_;
};
