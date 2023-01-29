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

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>
#include <fstream>
#include <regex>
#include <string>
#include <nlohmann/json.hpp>

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include "../common/nlohmann_json_wstring.h"
#include "../common/util.h"


namespace Settings {

	inline struct Launch {
		bool launch = false;
		std::wstring launchPath;
		std::wstring launchAppArgs;
		bool closeOnExit = true;
		bool waitForChildProcs = true;
		bool isUWP = false;
		bool ignoreLauncher = true;
		bool killLauncher = false;
		std::vector<std::wstring> launcherProcesses{};
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
		bool hideAltTab = true;
		bool disableGlosSIOverlay = false;
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
		std::wstring name;
		std::wstring icon;
		int version;
		std::wstring steamPath;
		std::wstring steamUserId;
		std::wstring standaloneModeGameId; /* = L"12605636929694728192"; */
		bool standaloneUseGamepadUI = false;
		bool allowStandAlone = true;
	} common;

	inline const std::map<std::wstring, std::function<void()>> cmd_args = {
		{L"-disableuwpoverlay", [&]() { common.no_uwp_overlay = true; }},
		{L"-disablewatchdog", [&]() { common.disable_watchdog = true; }},
		{L"-ignorelauncher", [&]() {  launch.ignoreLauncher = true; }},
		{L"-window", [&]() { window.windowMode = true; }},
		{L"-extendedLogging", [&]() { common.extendedLogging = true; }},
		{L"-standaloneUseGamepadUI", [&]() { common.standaloneUseGamepadUI = true; }},
		{L"-disallowStandAlone", [&]() { common.allowStandAlone = false; }},
	};

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

	inline void checkWinVer()
	{
		auto VN = util::win::GetRealOSVersion();
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
		constexpr auto safeParseValue = []<typename T>(const auto & object, const auto & key, T & value) {
			try {
				if (object.is_null() || object.empty() || object.at(key).empty() || object.at(key).is_null()) {
					return;
				}
				if constexpr (std::is_same_v<T, std::wstring>) {
					value = util::string::to_wstring(object[key].get<std::string>());
				}
				else {
					value = object[key];
				}
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

		int version;
		safeParseValue(json, "version", version);
		if (version != 1) { // TODO: versioning stuff
			spdlog::warn("Config version doesn't match application version.");
		}

		// TODO: make this as much generic as fits in about the same amount of code if one would parse every value separately.
		try {
			if (const auto launchconf = json["launch"]; !launchconf.is_null() && !launchconf.empty() && launchconf.is_object()) {
				safeParseValue(launchconf, "launch", launch.launch);
				safeParseValue(launchconf, "launchPath", launch.launchPath);
				safeParseValue(launchconf, "launchAppArgs", launch.launchAppArgs);
				safeParseValue(launchconf, "closeOnExit", launch.closeOnExit);
				safeParseValue(launchconf, "waitForChildProcs", launch.waitForChildProcs);
				safeParseValue(launchconf, "killLauncher", launch.killLauncher);
				safeParseValue(launchconf, "ignoreLauncher", launch.ignoreLauncher);

				if (const auto launcherProcs = launchconf["launcherProcesses"];
					!launcherProcs.is_null() && !launcherProcs.empty() && launcherProcs.is_array()) {
					launch.launcherProcesses.clear();
					launch.launcherProcesses.reserve(launcherProcs.size());
					for (auto& proc : launcherProcs) {
						launch.launcherProcesses.push_back(util::string::to_wstring(proc));
					}
				}
			}

			if (const auto devconf = json["devices"]; !devconf.is_null() && !devconf.empty() && devconf.is_object()) {
				safeParseValue(devconf, "hideDevices", devices.hideDevices);
				safeParseValue(devconf, "realDeviceIds", devices.realDeviceIds);
			}

			if (const auto winconf = json["window"]; !winconf.is_null() && !winconf.empty() && winconf.is_object()) {
				safeParseValue(winconf, "windowMode", window.windowMode);
				safeParseValue(winconf, "maxFps", window.maxFps);
				safeParseValue(winconf, "scale", window.scale);
				safeParseValue(winconf, "disableOverlay", window.disableOverlay);
				safeParseValue(winconf, "hideAltTab", window.hideAltTab);
				safeParseValue(winconf, "disableGlosSIOverlay", window.disableGlosSIOverlay);
			}

			if (const auto controllerConf = json["controller"]; !controllerConf.is_null() && !controllerConf.empty() && controllerConf.is_object()) {
				safeParseValue(controllerConf, "maxControllers", controller.maxControllers);
				safeParseValue(controllerConf, "allowDesktopConfig", controller.allowDesktopConfig);
				safeParseValue(controllerConf, "emulateDS4", controller.emulateDS4);
			}
			safeParseValue(json, "extendedLogging", common.extendedLogging);
			safeParseValue(json, "name", common.name);
			safeParseValue(json, "icon", common.icon);
			safeParseValue(json, "version", common.version);

			safeParseValue(json, "steamPath", common.steamPath);
			safeParseValue(json, "steamUserId", common.steamUserId);

			safeParseValue(json, "standaloneModeGameId", common.standaloneModeGameId);
			safeParseValue(json, "standaloneUseGamepadUI", common.standaloneUseGamepadUI);
		}
		catch (const nlohmann::json::exception& e) {
			spdlog::warn("Err parsing config: {}", e.what());
		}
		catch (const std::exception& e) {
			spdlog::warn("Err parsing config: {}", e.what());
		}
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
			if (cmd_args.contains(arg))
			{
				cmd_args.at(arg)();
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
		auto path = util::path::getDataDirPath();
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
		json["launch"]["launchPath"] = launch.launchPath;
		json["launch"]["launchAppArgs"] = launch.launchAppArgs;
		json["launch"]["closeOnExit"] = launch.closeOnExit;
		json["launch"]["waitForChildProcs"] = launch.waitForChildProcs;
		json["devices"]["hideDevices"] = devices.hideDevices;
		json["devices"]["realDeviceIds"] = devices.realDeviceIds;
		json["window"]["windowMode"] = window.windowMode;
		json["window"]["maxFps"] = window.maxFps;
		json["window"]["scale"] = window.scale;
		json["window"]["disableOverlay"] = window.disableOverlay;
		json["window"]["hideAltTab"] = window.hideAltTab;
		json["controller"]["maxControllers"] = controller.maxControllers;
		json["controller"]["allowDesktopConfig"] = controller.allowDesktopConfig;
		json["controller"]["emulateDS4"] = controller.emulateDS4;

		json["extendedLogging"] = common.extendedLogging;
		json["name"] = common.name;
		json["icon"] = common.icon;
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
