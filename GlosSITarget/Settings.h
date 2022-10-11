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

#include <fstream>
#include <regex>
#include <string>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <locale>
#include <codecvt>

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <KnownFolders.h>
#endif

namespace Settings {

inline struct Launch {
    bool launch = false;
    std::wstring launchPath;
    std::wstring launchAppArgs;
    bool closeOnExit = true;
    bool waitForChildProcs = true;
    bool isUWP = false;
} launch;

inline struct Devices {
    bool hideDevices = true;
    bool realDeviceIds = false;
} devices;

inline struct Window {
    bool windowMode = false;
    int maxFps = 0;
    float scale = 0.f;
    bool disableOverlay = false;
} window;

inline struct Controller {
    int maxControllers = 1;
    bool allowDesktopConfig = false;
    bool emulateDS4 = false;
} controller;

inline struct Common {
    bool no_uwp_overlay = false;
    bool disable_watchdog = false;
    bool extendedLogging = false;
    bool ignoreEGS = true;
    std::wstring name;
    std::wstring icon;
    int version;
} common;

inline std::filesystem::path settings_path_ = "";

inline bool checkIsUwp(const std::wstring& launch_path)
{
    if (launch_path.find(L"://") != std::wstring::npos) {
        return false;
    }
    std::wsmatch m;
    if (!std::regex_search(launch_path, m, std::wregex(L"^.{1,5}:"))) {
        return true;
    }
    return false;
}

#ifdef WIN32
inline bool isWin10 = false;

typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

inline RTL_OSVERSIONINFOW GetRealOSVersion()
{
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            RTL_OSVERSIONINFOW rovi = {0};
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (STATUS_SUCCESS == fxPtr(&rovi)) {
                return rovi;
            }
        }
    }
    RTL_OSVERSIONINFOW rovi = {0};
    return rovi;
}

inline void checkWinVer()
{
    auto VN = GetRealOSVersion();
    isWin10 = VN.dwBuildNumber < 22000;

    if (isWin10) {
        spdlog::info("Running on Windows 10; Winver: {}.{}.{}", VN.dwMajorVersion, VN.dwMinorVersion, VN.dwBuildNumber);
    }
    else {
        spdlog::info("Running on Windows 11; Winver: {}.{}.{}", VN.dwMajorVersion, VN.dwMinorVersion, VN.dwBuildNumber);
    }
}
#endif

inline void Parse(const nlohmann::basic_json<>& json)
{
    auto safeParseValue = [](const auto& object, const auto& key, auto& value) {
        try {
            if (object.is_null() || object.empty() || object.at(key).empty() || object.at(key).is_null()) {
                return;
            }
            value = object[key];
        }
        catch (const nlohmann::json::exception& e) {
            e.id == 403
                ? spdlog::trace("Err parsing \"{}\"; {}", key, e.what())
                : spdlog::warn("Err parsing \"{}\"; {}", key, e.what());
        }
        catch (const std::exception& e) {
            spdlog::warn("Err parsing \"{}\"; {}", key, e.what());
        }
    };

    auto safeWStringParse = [&safeParseValue](const auto& object, const auto& key, std::wstring& value) {
        std::string meh;
        safeParseValue(object, key, meh);
        if (!meh.empty()) {
            // This assumes that char is utf8 and wchar_t is utf16, which is guaranteed on Windows.
            value = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(meh);
        }
    };

    int version;
    safeParseValue(json, "version", version);
    if (version != 1) { // TODO: versioning stuff
        spdlog::warn("Config version doesn't match application version.");
    }

    // TODO: make this as much generic as fits in about the same amount of code if one would parse every value separately.
    try {
        if (auto launchconf = json["launch"]; !launchconf.is_null() && !launchconf.empty() && launchconf.is_object()) {
            safeParseValue(launchconf, "launch", launch.launch);
            safeWStringParse(launchconf, "launchPath", launch.launchPath);
            safeWStringParse(launchconf, "launchAppArgs", launch.launchAppArgs);
            safeParseValue(launchconf, "closeOnExit", launch.closeOnExit);
            safeParseValue(launchconf, "waitForChildProcs", launch.waitForChildProcs);
        }

        if (auto devconf = json["devices"]; !devconf.is_null() && !devconf.empty() && devconf.is_object()) {
            safeParseValue(devconf, "hideDevices", devices.hideDevices);
            safeParseValue(devconf, "realDeviceIds", devices.realDeviceIds);
        }

        if (auto winconf = json["window"]; !winconf.is_null() && !winconf.empty() && winconf.is_object()) {
            safeParseValue(winconf, "windowMode", window.windowMode);
            safeParseValue(winconf, "maxFps", window.maxFps);
            safeParseValue(winconf, "scale", window.scale);
            safeParseValue(winconf, "disableOverlay", window.disableOverlay);
        }

        if (auto controllerConf = json["controller"]; !controllerConf.is_null() && !controllerConf.empty() && controllerConf.is_object()) {
            safeParseValue(controllerConf, "maxControllers", controller.maxControllers);
            safeParseValue(controllerConf, "allowDesktopConfig", controller.allowDesktopConfig);
            safeParseValue(controllerConf, "emulateDS4", controller.emulateDS4);
        }
    }
    catch (const nlohmann::json::exception& e) {
        spdlog::warn("Err parsing config: {}", e.what());
    }
    catch (const std::exception& e) {
        spdlog::warn("Err parsing config: {}", e.what());
    }

    safeParseValue(json, "extendedLogging", common.extendedLogging);
    safeWStringParse(json, "name", common.name);
    safeWStringParse(json, "icon", common.icon);
    safeParseValue(json, "version", common.version);
    safeParseValue(json, "ignoreEGS", common.ignoreEGS);

    if (launch.launch) {
        launch.isUWP = checkIsUwp(launch.launchPath);
    }
}

inline void Parse(const std::vector<std::wstring>& args)
{
    std::wstring configName;
    for (const auto& arg : args) {
        if (arg.empty()) {
            continue;
        }
        if (arg == L"-disableuwpoverlay") {
            common.no_uwp_overlay = true;
        }
        else if (arg == L"-disablewatchdog") {
            common.disable_watchdog = true;
        }
        else if (arg == L"-ignoreegs") {
            common.ignoreEGS = true;
        }
        else {
            configName += L" " + std::wstring(arg.begin(), arg.end());
        }
    }
    if (!configName.empty()) {
        if (configName[0] == L' ') {
            configName.erase(configName.begin());
        }
        if (!configName.ends_with(L".json")) {
            configName += L".json";
        }
    }
    wchar_t* localAppDataFolder;
    std::filesystem::path path;
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) != S_OK) {
        path = std::filesystem::temp_directory_path().parent_path().parent_path().parent_path();
    }
    else {
        path = std::filesystem::path(localAppDataFolder).parent_path();
    }

    path /= "Roaming";
    path /= "GlosSI";
    if (!configName.empty()) {
        path /= "Targets";
        path /= configName;
    }
    else {
        spdlog::info("No config file specified, using default");
        path /= "default.json";
    }

    std::ifstream json_file;
    json_file.open(path);
    if (!json_file.is_open()) {
        spdlog::error(L"Couldn't open settings file {}", path.wstring());
        spdlog::debug(L"Using sane defaults...");
        return;
    }
    settings_path_ = path;
    const auto& json = nlohmann::json::parse(json_file);
    Parse(json);

    spdlog::debug("Read config file \"{}\"; config: {}", path.string(), json.dump());
    json_file.close();
}

inline nlohmann::json toJson()
{
    nlohmann::json json;
    json["version"] = 1;
    json["launch"]["launch"] = launch.launch;
    json["launch"]["launchPath"] = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(launch.launchPath);
    json["launch"]["launchAppArgs"] = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(launch.launchAppArgs);
    json["launch"]["closeOnExit"] = launch.closeOnExit;
    json["launch"]["waitForChildProcs"] = launch.waitForChildProcs;
    json["devices"]["hideDevices"] = devices.hideDevices;
    json["devices"]["realDeviceIds"] = devices.realDeviceIds;
    json["window"]["windowMode"] = window.windowMode;
    json["window"]["maxFps"] = window.maxFps;
    json["window"]["scale"] = window.scale;
    json["window"]["disableOverlay"] = window.disableOverlay;
    json["controller"]["maxControllers"] = controller.maxControllers;
    json["controller"]["allowDesktopConfig"] = controller.allowDesktopConfig;
    json["controller"]["emulateDS4"] = controller.emulateDS4;

    json["extendedLogging"] = common.extendedLogging;
    json["name"] = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(common.name);
    json["icon"] = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(common.icon);
    json["version"] = common.version;
    return json;
}

inline void StoreSettings()
{
    const auto& json = toJson();

    std::ofstream json_file;
    json_file.open(settings_path_);
    if (!json_file.is_open()) {
        spdlog::error(L"Couldn't open settings file {}", settings_path_.wstring());
        return;
    }
    json_file << json.dump(4);
    json_file.close();
}

} // namespace Settings
