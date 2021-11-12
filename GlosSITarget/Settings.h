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

#include <fstream>
#include <regex>
#include <string>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace Settings {


inline struct Launch {
    bool launch = false;
    std::wstring launchPath;
    std::wstring launchAppArgs;
    bool closeOnExit = true;
    bool isUWP = false;
} launch;

inline struct Devices {
    bool hideDevices = true;
} devices;

inline struct Window {
    bool windowMode = false;
    int maxFps = 0;
    float scale = 0.f;
} window;

inline bool checkIsUwp(const std::wstring& launch_path)
{
    std::wsmatch m;
    if (!std::regex_search(launch_path, m, std::wregex(L"^.{1,3}:"))) {
        return true;
    }
    return false;
}

inline void Parse(std::string arg1)
{
    if (!arg1.ends_with(".json")) {
        arg1 += ".json";
    }
    std::filesystem::path path(arg1);
    if (path.has_extension() && !std::filesystem::exists(path)) {
        path = std::filesystem::temp_directory_path()
                        .parent_path()
                        .parent_path()
                        .parent_path();

        path /= "Roaming";
        path /= "GlosSI";
        path /= "Targets";
        path /= arg1;
    }
    std::ifstream json_file;
    json_file.open(path);
    if (!json_file.is_open()) {
        spdlog::error("Couldn't open settings file {}", path.string());
        return;
    }
    const auto json = nlohmann::json::parse(json_file);
    if (json["version"] != 1) { // TODO: versioning stuff
        spdlog::warn("Config version doesn't match application version.");
    }

    // TODO: make this as much generic as fits in about the same amount of code if one would parse every value separately.

    auto safeParseValue = [](const auto& object, const auto& key, auto& value) {
        try {
            if (object[key].is_null()) {
                return;
            }
            value = object[key];
        } catch (const nlohmann::json::exception& e) {
            spdlog::error("Err parsing \"{}\"; {}", key, e.what());
        }
    };

    auto safeWStringParse = [&safeParseValue](const auto& object, const auto& key, std::wstring& value) {
        std::string meh;
        safeParseValue(object, key, meh);
        if (!meh.empty()) {
            value.clear();
            std::ranges::transform(meh, std::back_inserter(value), [](const auto& ch) {
                return static_cast<wchar_t>(ch);
            });
        }
    };

    if (auto launchconf = json["launch"]; launchconf.is_object()) {
        safeParseValue(launchconf, "launch", launch.launch);
        safeWStringParse(launchconf, "launchPath", launch.launchPath);
        safeWStringParse(launchconf, "launchAppArgs", launch.launchAppArgs);
        safeParseValue(launchconf, "closeOnExit", launch.closeOnExit);
    }

    if (auto devconf = json["devices"]; devconf.is_object()) {
        safeParseValue(devconf, "hideDevices", devices.hideDevices);
    }

    if (auto winconf = json["window"]; winconf.is_object()) {
        safeParseValue(winconf, "windowMode", window.windowMode);
        safeParseValue(winconf, "maxFps", window.maxFps);
        safeParseValue(winconf, "scale", window.scale);
    }

    json_file.close();

    spdlog::debug("Read config file \"{}\"", path.string());

    if (launch.launch) {
        launch.isUWP = checkIsUwp(launch.launchPath);
    }

}

} // namespace Settings
