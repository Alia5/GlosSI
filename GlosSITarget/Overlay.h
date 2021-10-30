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
#include <string>
#include <SFML/Graphics.hpp>

#define IMGUI_USER_CONFIG "imconfig.h"
#include "imgui-SFML.h"
#include "imgui.h"

#include <chrono>

#include <spdlog/spdlog.h>

class Overlay {
  public:
    Overlay(sf::RenderWindow& window, std::function<void()> on_close, bool force_enable = false);

    void setEnabled(bool enabled);
    bool isEnabled() const;
    bool toggle();
    void update();
    static void ProcessEvent(sf::Event evnt);
    static void Shutdown();
    static void AddLog(const spdlog::details::log_msg& msg);

    static int AddOverlayElem(const std::function<void()>& elem_fn);
    static void RemoveOverlayElem(int id);

  private:
    sf::RenderWindow& window_;
    sf::Clock update_clock_;
    bool enabled_ = true;
    std::function<void()> on_close_;
    void showLogs() const;
    [[nodiscard]] bool closeButton() const;
    bool force_enable_ = false;

    struct Log {
        std::chrono::system_clock::time_point time;
        spdlog::level::level_enum level;
        std::string payload;
    };
    static inline std::vector<Log> LOG_MSGS_;
    static constexpr int LOG_RETENTION_TIME_ = 5;

    static inline int overlay_element_id_ = 0;
    static inline std::map<int, std::function<void()>> OVERLAY_ELEMS_;

#ifdef _WIN32
    std::string config_file_name_;
#endif

};
