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
#include "Overlay.h"

#include <filesystem>
#include <utility>

Overlay::Overlay(sf::RenderWindow& window, std::function<void()> on_close) : window_(window), on_close_(std::move(on_close))
{
    ImGui::SFML::Init(window_);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

#ifdef _WIN32
    auto config_path = std::filesystem::temp_directory_path()
                           .parent_path()
                           .parent_path()
                           .parent_path();

    config_path /= "Roaming";
    config_path /= "GlosSI";
    if (!std::filesystem::exists(config_path))
        std::filesystem::create_directories(config_path);
    config_path /= "imgui.ini";
    config_file_name_ = config_path.string();
    io.IniFilename = config_file_name_.data();
#endif

    window.resetGLStates();

    //Hack: Trick ImGui::SFML into thinking we already have focus (otherwise only notices after focus-lost)
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
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
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

    showLogs();

    if (enabled_) {
        window_.clear(sf::Color(0, 0, 0, 64)); // make window slightly dim screen with overlay

        std::ranges::for_each(OVERLAY_ELEMS_, [](const auto& fn) { fn(); });

        // ImGui::ShowDemoWindow();

        if (closeButton()) {
            return;
        }
    }

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

void Overlay::AddOverlayElem(const std::function<void()>& elem_fn)
{
    OVERLAY_ELEMS_.push_back(elem_fn);
}

void Overlay::showLogs() const
{
    std::vector<Log> logs;
    if (enabled_) {
        logs = LOG_MSGS_;
    }
    else {
        std::ranges::copy_if(LOG_MSGS_,
                             std::back_inserter(logs),
                             [](const auto& log) {
                                 return log.time.time_since_epoch() + std::chrono::seconds(LOG_RETENTION_TIME_) > std::chrono::system_clock::now().time_since_epoch();
                             });
    }
    if (logs.empty())
        return;
    if (!enabled_) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {32.f, 32.f});
        ImGui::Begin("Log", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    }
    else {
        ImGui::Begin("Log", nullptr);
    }
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
    ImGui::End();
    if (!enabled_) {
        ImGui::PopStyleVar();
    }
}

bool Overlay::closeButton() const
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.f, 0.f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.6f, 0.f, 0.f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.16f, 0.16f, 1.00f));
    ImGui::Begin("##CloseButton", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize({56 + 24, 32 + 24});
    ImGui::SetWindowPos({window_.getSize().x - ImGui::GetWindowWidth() + 24, -24});
    if (ImGui::Button("X##Close", {56, 48})) {
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
