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
    Overlay(sf::RenderWindow& window, std::function<void()> on_close, std::function<void()> trigger_state_change, bool force_enable = false);

    void setEnabled(bool enabled);
    bool isEnabled() const;
    bool toggle();
    void update();
    static void ProcessEvent(sf::Event evnt);
    static void Shutdown();
    static void AddLog(const spdlog::details::log_msg& msg);

    static int AddOverlayElem(const std::function<void(bool window_has_focus, ImGuiID dockspace_id)>& elem_fn, bool force_show_ = false);
    static void RemoveOverlayElem(int id);

  private:
    sf::RenderWindow& window_;
    sf::Clock update_clock_;
    bool enabled_ = true;
    std::function<void()> on_close_;
    std::function<void()> trigger_state_change_;
    void showLogs(ImGuiID dockspace_id);
    bool closeOverlayButton() const;
    [[nodiscard]] bool closeButton() const;
    void showSplash(uint8_t alpha = 128);
    bool force_enable_ = false;
    bool log_expanded_ = true;
    sf::Clock time_since_start_clock_;

    sf::Texture logo_texture_;
    sf::Sprite logo_sprite_;

    struct Log {
        std::chrono::system_clock::time_point time;
        spdlog::level::level_enum level;
        std::string payload;
    };
    static inline std::vector<Log> LOG_MSGS_;
    static constexpr int LOG_RETENTION_TIME_ = 5;
    static constexpr int HIDE_NORMAL_LOGS_AFTER_S = 20;
    static constexpr int SPLASH_DURATION_S_ = 3;

    static inline int overlay_element_id_ = 0;
    static inline std::map<int, std::function<void(bool window_has_focus, ImGuiID dockspace_id)>> OVERLAY_ELEMS_;

    static inline std::map<int, std::function<void(bool window_has_focus, ImGuiID dockspace_id)>> FORCED_OVERLAY_ELEMS_;

#ifdef _WIN32
    std::string config_file_name_;
#endif
};
