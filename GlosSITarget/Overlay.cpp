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
#include "Overlay.h"

#include <filesystem>
#include <utility>
#include <regex>
#include <shlobj_core.h>

#include "Roboto.h"
#include "..\common\Settings.h"
#include "GlosSI_logo.h"

#include "../version.hpp"

Overlay::Overlay(
    sf::RenderWindow& window,
    std::function<void()> on_close,
    std::function<void()> trigger_state_change,
    bool force_enable)
    : window_(window),
      on_close_(std::move(on_close)),
      trigger_state_change_(std::move(trigger_state_change)),
      force_enable_(force_enable)
{
    ImGui::SFML::Init(window_);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.Fonts->Clear(); // clear fonts if you loaded some before (even if only default one was loaded)
    auto fontconf = ImFontConfig{};
    fontconf.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf.data(), Roboto_Regular_ttf.size(), 24, &fontconf);
    ImGui::SFML::UpdateFontTexture();

#ifdef _WIN32
    auto config_path = util::path::getDataDirPath();
    config_path /= "imgui.ini";

    // This assumes that char is utf8 and wchar_t is utf16, which is guaranteed on Windows.
    config_file_name_ = util::string::to_string(config_path.wstring());
    io.IniFilename = config_file_name_.data();
#endif

    if (!logo_texture_.loadFromMemory(GLOSSI_LOGO.data(), GLOSSI_LOGO.size())) {
        spdlog::trace("Failed to load logo texture");
    }
    if (logo_texture_.getSize().x > 0) {
        logo_sprite_.setTexture(logo_texture_);
        logo_sprite_.setScale(0.5f, 0.5f);
    }

    window.resetGLStates();

    // Hack: Trick ImGui::SFML into thinking we already have focus (otherwise only notices after focus-lost)
    const sf::Event ev(sf::Event::GainedFocus);
    ProcessEvent(ev);

    auto& style = ImGui::GetStyle();
    style.WindowBorderSize = 0;
    style.WindowRounding = 12;
    style.ScrollbarRounding = 12;
    style.PopupRounding = 12;
    style.ChildRounding = 5;
    style.ScrollbarRounding = 12;
    style.GrabRounding = 5;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.13f, 0.14f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 0.05f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.21f, 0.24f, 0.94f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.26f, 0.29f, 0.34f, 0.93f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.19f, 0.22f, 0.81f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.20f, 0.25f, 0.95f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.29f, 0.54f, 0.83f, 0.96f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void Overlay::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

bool Overlay::isEnabled() const
{
    return enabled_;
}

bool Overlay::toggle()
{
    enabled_ = !enabled_;
    return enabled_;
}

void Overlay::update()
{
    ImGui::SFML::Update(window_, update_clock_.restart());

    if (!enabled_ && !force_enable_ && time_since_start_clock_.getElapsedTime().asSeconds() < SPLASH_DURATION_S_) {
        const auto millis = time_since_start_clock_.getElapsedTime().asMilliseconds();
        const auto fade_millis = 1000;
        const auto remain_millis = SPLASH_DURATION_S_ * 1000 - millis;
        if (remain_millis <= fade_millis) {
            showSplash(static_cast<float>(remain_millis) / static_cast<float>(fade_millis) * 128.f);
        }
        else {
            showSplash(128);
        }
    }

    if (Settings::window.disableGlosSIOverlay) {
        std::ranges::for_each(FORCED_OVERLAY_ELEMS_, [this](const auto& elem) {
            elem.second(window_.hasFocus(), 0);
        });
        ImGui::SFML::Render(window_);
        return;
    }

    showLogs(0);

    if (enabled_ || force_enable_) {

        showSplash();

        // Create a DockSpace node where any window can be docked
        ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.6f, ImGui::GetMainViewport()->Size.y * 0.7f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({ImGui::GetMainViewport()->Size.x * 0.25f, 100}, ImGuiCond_FirstUseEver);
        ImGui::Begin("GlosSI Settings");
        ImGui::Text("Version: %s", version::VERSION_STR);
        if (Settings::settings_path_ != "") {
            ImGui::SameLine();
            if (ImGui::Button("Save shortcut settings", {256 * ImGui::GetIO().FontGlobalScale, 32 * ImGui::GetIO().FontGlobalScale})) {
                Settings::StoreSettings();
            }
        }
        ImGui::Checkbox("Extended logging", &Settings::common.extendedLogging);
        ImGuiID dockspace_id = ImGui::GetID("GlosSI-DockSpace");
        ImGui::DockSpace(dockspace_id);

        window_.clear(sf::Color(0, 0, 0, 128)); // make window slightly dim screen with overlay

        std::ranges::for_each(OVERLAY_ELEMS_, [this, &dockspace_id](const auto& elem) {
            elem.second(window_.hasFocus(), dockspace_id);
        });

        ImGui::End();

        // ImGui::ShowDemoWindow();

        if (closeButton()) {
            return;
        }

        closeOverlayButton();
    }

    std::ranges::for_each(FORCED_OVERLAY_ELEMS_, [this](const auto& elem) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.07f, 0.95f));
        ImGui::SetNextWindowPos({
            ImGui::GetMainViewport()->Size.x * 0.5f,
            ImGui::GetMainViewport()->Size.y * 0.5f
        }, ImGuiCond_Always, 
            {0.5f, 0.5f});
        elem.second(window_.hasFocus(), 0);
        ImGui::PopStyleColor();
    });

    ImGui::SFML::Render(window_);
}

void Overlay::ProcessEvent(sf::Event evnt)
{
    ImGui::SFML::ProcessEvent(evnt);
}

void Overlay::Shutdown()
{
    ImGui::SFML::Shutdown();
}

void Overlay::AddLog(const spdlog::details::log_msg& msg)
{
    LOG_MSGS_.push_back({.time = msg.time, .level = msg.level, .payload = msg.payload.data()});
}

int Overlay::AddOverlayElem(const std::function<void(bool window_has_focus, ImGuiID dockspace_id)>& elem_fn, bool force_show)
{
    if (force_show) {
        FORCED_OVERLAY_ELEMS_.insert({overlay_element_id_, elem_fn});
    }
    else {
        OVERLAY_ELEMS_.insert({overlay_element_id_, elem_fn});
    }
    // keep this non confusing, but longer...
    const auto res = overlay_element_id_;
    overlay_element_id_++;
    return res;
}

void Overlay::RemoveOverlayElem(int id)
{
    if (OVERLAY_ELEMS_.contains(id))
        OVERLAY_ELEMS_.erase(id);
    if (FORCED_OVERLAY_ELEMS_.contains(id))
        FORCED_OVERLAY_ELEMS_.erase(id);
}

void Overlay::showLogs(ImGuiID dockspace_id)
{
    std::vector<Log> logs;
    if (!enabled_ && !log_expanded_) {
        return;
    }
    bool logs_contain_warn_or_worse = false;
    if (enabled_) {
        logs = LOG_MSGS_;
    }
    else {
        std::ranges::copy_if(LOG_MSGS_,
                             std::back_inserter(logs),
                             [&logs_contain_warn_or_worse](const auto& log) {
                                 const auto res = (log.time.time_since_epoch() + std::chrono::seconds(
                                                                                     LOG_RETENTION_TIME_) >
                                                   std::chrono::system_clock::now().time_since_epoch())
#ifdef NDEBUG
                                                  && (log.level > spdlog::level::debug)
#endif
                                     ;
                                 if (res && log.level > spdlog::level::warn) {
                                     logs_contain_warn_or_worse = true;
                                 }
                                 return res;
                             });
    }
    if (logs.empty() || (!enabled_ && !force_enable_ && !logs_contain_warn_or_worse && time_since_start_clock_.getElapsedTime().asSeconds() > HIDE_NORMAL_LOGS_AFTER_S))
        return;
    ImGui::SetNextWindowSizeConstraints({150, 150}, {1000, window_.getSize().y - 250.f});
    if (!enabled_) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {32.f, 32.f});
        ImGui::Begin("Log", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    }
    else {
        // ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.2f, ImGui::GetMainViewport()->Size.y * 0.7f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({ImGui::GetMainViewport()->Size.x * 0.05f, 100}, ImGuiCond_FirstUseEver);
        log_expanded_ = ImGui::Begin("Log");
    }
    if (log_expanded_) {
        std::ranges::for_each(LOG_MSGS_, [](const auto& msg) {
            switch (msg.level) {
            case spdlog::level::warn:
                ImGui::TextColored({1.f, 0.8f, 0.f, 1.f}, msg.payload.data());
                break;
            case spdlog::level::err:
                ImGui::TextColored({.8f, 0.0f, 0.f, 1.f}, msg.payload.data());
                break;
            case spdlog::level::debug:
                ImGui::TextColored({.8f, 0.8f, 0.8f, .9f}, msg.payload.data());
                break;
            default:
                ImGui::Text(msg.payload.data());
            }
        });
        ImGui::SetScrollY(ImGui::GetScrollMaxY());
    }
    ImGui::End();
    if (!enabled_) {
        ImGui::PopStyleVar();
    }
}

bool Overlay::closeOverlayButton() const
{
    if (force_enable_) {
        return false;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.f, 0.f, 0.0f));
    ImGui::Begin("##CloseOverlayButton", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos({(window_.getSize().x - ImGui::GetWindowWidth()) / 2, 32});
    ImGui::SetWindowSize({256 * ImGui::GetIO().FontGlobalScale, 32 * ImGui::GetIO().FontGlobalScale});
    if (ImGui::Button("Return to Game", {256 * ImGui::GetIO().FontGlobalScale, 32 * ImGui::GetIO().FontGlobalScale})) {
        trigger_state_change_();
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        return true;
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    return false;
}

bool Overlay::closeButton() const
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.f, 0.f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.6f, 0.f, 0.f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.16f, 0.16f, 1.00f));
    ImGui::Begin("##CloseButton", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize({(56 + 32) * ImGui::GetIO().FontGlobalScale, (32 + 32) * ImGui::GetIO().FontGlobalScale});
    ImGui::SetWindowPos({window_.getSize().x - ImGui::GetWindowWidth() + 32, -32});
    if (ImGui::Button("X##Close", {56 * ImGui::GetIO().FontGlobalScale, 32 * ImGui::GetIO().FontGlobalScale})) {
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        on_close_();
        return true;
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    return false;
}

void Overlay::showSplash(uint8_t alpha)
{
    if (logo_texture_.getSize().x > 0) {
        ImGui::SetNextWindowPos(
            {ImGui::GetMainViewport()->Size.x * 0.5f - static_cast<float>(logo_texture_.getSize().x) * 0.5f * logo_sprite_.getScale().x,
             ImGui::GetMainViewport()->Size.y * 0.5f - static_cast<float>(logo_texture_.getSize().y) * 0.5f * logo_sprite_.getScale().y});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.f, 0.f, 0.0f));
        ImGui::Begin("##Splash", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
        ImGui::Image(logo_sprite_, {255, 255, 255, alpha});
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
}
