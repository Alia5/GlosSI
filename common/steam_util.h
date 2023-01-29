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
#include <WinReg/WinReg.hpp>
#include <vdf_parser.hpp>


#include "util.h"
#include "Settings.h"

namespace util
{
	namespace steam
	{

		static constexpr std::wstring_view user_data_path = L"/userdata/";
		static constexpr std::wstring_view config_file_name = L"/config/localconfig.vdf";
		static constexpr std::string_view overlay_hotkey_name = "InGameOverlayShortcutKey ";
		static constexpr std::string_view screenshot_hotkey_name = "InGameOverlayScreenshotHotKey ";

		inline std::filesystem::path getSteamPath()
		{
#ifdef _WIN32
			try {
				// TODO: check if keys/value exist
				// steam should always be open and have written reg values...
				winreg::RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam" };
				const auto res = key.GetStringValue(L"SteamPath");
				spdlog::info(L"Detected Steam Path: {}", res);
				return res;
			}
			catch (const winreg::RegException& e) {
				spdlog::error("Couldn't get Steam path from Registry; {}", e.what());
			}
			return Settings::common.steamPath;
#else
			return L""; // TODO
#endif
		}

		inline std::wstring getSteamUserId()
		{
#ifdef _WIN32
			try {
				// TODO: check if keys/value exist
				// steam should always be open and have written reg values...
				winreg::RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess" };
				const auto res = std::to_wstring(key.GetDwordValue(L"ActiveUser"));
				spdlog::info(L"Detected Steam UserId: {}", res);
				return res;
			}
			catch (const winreg::RegException& e) {
				spdlog::error("Couldn't get Steam path from Registry; {}", e.what());
			}
			return Settings::common.steamUserId;
#else
			return L""; // TODO
#endif
		}

		inline std::vector<std::string> getOverlayHotkey(const std::wstring& steam_path = getSteamPath(), const std::wstring& steam_user_id = getSteamPath())
		{
			const auto config_path = std::wstring(steam_path) + std::wstring(user_data_path) + steam_user_id + std::wstring(config_file_name);
			if (!std::filesystem::exists(config_path)) {
				spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
				return { "Shift", "KEY_TAB" }; // default
			}
			std::ifstream config_file(config_path);
			auto root = tyti::vdf::read(config_file);

			std::shared_ptr<tyti::vdf::basic_object<char>> children = root.childs["system"];
			if (!children || children->attribs.empty() || !children->attribs.contains("InGameOverlayShortcutKey")) {
				spdlog::warn("Couldn't detect overlay hotkey, using default: Shift+Tab");
				return { "Shift", "KEY_TAB" }; // default
			}
			auto hotkeys = children->attribs.at("InGameOverlayShortcutKey");

			// has anyone more than 4 keys to open overlay?!
			std::smatch m;
			if (!std::regex_match(hotkeys, m, std::regex(R"((\w*)\s*(\w*)\s*(\w*)\s*(\w*))"))) {
				spdlog::warn("Couldn't detect overlay hotkey, using default: Shift+Tab");
				return { "Shift", "KEY_TAB" }; // default
			}

			std::vector<std::string> res;
			for (auto i = 1; i < m.size(); i++) {
				const auto s = std::string(m[i]);
				if (!s.empty()) {
					res.push_back(s);
				}
			}
			if (res.empty()) {
				spdlog::warn("Couldn't detect overlay hotkey, using default: Shift+Tab");
				return { "Shift", "KEY_TAB" }; // default
			}
			spdlog::info("Detected Overlay hotkey(s): {}", std::accumulate(
				res.begin() + 1, res.end(), res[0],
				[](auto acc, const auto curr) { return acc += "+" + curr; }));
			return res;
		}

		inline std::vector<std::string> getScreenshotHotkey(const std::wstring& steam_path = getSteamPath(), const std::wstring& steam_user_id = getSteamPath())
		{
			const auto config_path = std::wstring(steam_path) + std::wstring(user_data_path) + steam_user_id + std::wstring(config_file_name);
			if (!std::filesystem::exists(config_path)) {
				spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
				return { "KEY_F12" }; // default
			}
			std::ifstream config_file(config_path);
			auto root = tyti::vdf::read(config_file);

			std::shared_ptr<tyti::vdf::basic_object<char>> children = root.childs["system"];
			if (!children || children->attribs.empty() || !children->attribs.contains("InGameOverlayScreenshotHotKey")) {
				spdlog::warn("Couldn't detect overlay hotkey, using default: F12");
				return { "KEY_F12" }; // default
			}
			auto hotkeys = children->attribs.at("InGameOverlayScreenshotHotKey");

			// has anyone more than 4 keys to screenshot?!
			std::smatch m;
			if (!std::regex_match(hotkeys, m, std::regex(R"((\w*)\s*(\w*)\s*(\w*)\s*(\w*))"))) {
				spdlog::warn("Couldn't detect overlay hotkey, using default: F12");
				return { "KEY_F12" }; // default
			}

			std::vector<std::string> res;
			for (auto i = 1; i < m.size(); i++) {
				const auto s = std::string(m[i]);
				if (!s.empty()) {
					res.push_back(s);
				}
			}
			if (res.empty()) {
				spdlog::warn("Couldn't detect overlay hotkey, using default: F12");
				return { "KEY_F12" }; // default
			}
			spdlog::info("Detected screenshot hotkey(s): {}", std::accumulate(
				res.begin() + 1, res.end(), res[0],
				[](auto acc, const auto curr) { return acc += "+" + curr; }));
			return res;
		}

		inline bool getXBCRebindingEnabled(const std::wstring& steam_path = getSteamPath(), const std::wstring& steam_user_id = getSteamPath())
		{
			const auto config_path = std::wstring(steam_path) + std::wstring(user_data_path) + steam_user_id + std::wstring(config_file_name);
			if (!std::filesystem::exists(config_path)) {
				spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
				return false;
			}
			std::ifstream config_file(config_path);
			auto root = tyti::vdf::read(config_file);

			if (root.attribs.empty() || !root.attribs.contains("SteamController_XBoxSupport")) {
				spdlog::warn("\"Xbox Configuration Support\" is disabled in Steam. This may cause doubled Inputs!");
				return false;
			}
			auto xbsup = root.attribs.at("SteamController_XBoxSupport");
			if (xbsup != "1") {
				spdlog::warn("\"Xbox Configuration Support\" is disabled in Steam. This may cause doubled Inputs!");
			}
			return xbsup == "1";
		}

		inline nlohmann::json getSteamConfig(const std::wstring& steam_path = getSteamPath(), const std::wstring& steam_user_id = getSteamUserId())
		{
			const auto config_path = std::wstring(steam_path) + std::wstring(user_data_path) + steam_user_id + std::wstring(config_file_name);
			if (!std::filesystem::exists(config_path)) {
				spdlog::warn(L"Couldn't read Steam config file: \"{}\"", config_path);
				return nlohmann::json();
			}
			std::ifstream config_file(config_path);
			auto root = tyti::vdf::read(config_file);
			if (root.attribs.empty())
			{
				return {};
			}
			auto res = nlohmann::json::object();
			res[root.name] = nlohmann::json::object();
			for (auto& [key, value] : root.attribs)
			{
				res[root.name][key] = value;
			}
			auto parse_child = [](nlohmann::json& j, std::shared_ptr<tyti::vdf::basic_object<char>> child, auto&& recurse) -> void
			{
				for (auto& [key, value] : child->attribs)
				{
					j[key] = value;
					for (auto& [childkey, childval] : child->childs)
					{
						j[childkey] = {};
						recurse(j[childkey], childval, recurse);
					}
				}
			};
			for (auto& [key, value] : root.childs)
			{
				res[root.name][key] = {};
				parse_child(res[root.name][key], value, parse_child);
			}
			return res;
		}

	}
}