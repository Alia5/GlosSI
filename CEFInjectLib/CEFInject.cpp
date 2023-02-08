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

#include "CEFInject.h"


#include <easywsclient.hpp>

#define _SSIZE_T_DEFINED
#include <easywsclient.cpp> // seems like a hack to me, but eh, what is in the doc will be done ¯\_(ツ)_/¯

#include "../common/nlohmann_json_wstring.h"

#include <fstream>
#include <streambuf>

#include <spdlog/spdlog.h>

#include "../common/Settings.h"

namespace CEFInject
{
	namespace internal
	{
		httplib::Client GetHttpClient(uint16_t port)
		{
			httplib::Client cli("localhost", port);
			cli.set_connection_timeout(0, 200000);
			cli.set_read_timeout(0, 500000);
			return cli;
		}

		static uint32_t msg_id = 0;

	}
	bool CEFDebugAvailable(uint16_t port)
	{
		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				return true;
			}
		}
		return false;
	}

	std::vector<std::wstring> AvailableTabNames(uint16_t port)
	{
		std::vector<std::wstring> tabs;
		const auto json = AvailableTabs(port);
		for (const auto& j : json) {
			tabs.push_back(j["title"].get<std::wstring>());
		}
		return tabs;
	}

	nlohmann::basic_json<> AvailableTabs(uint16_t port)
	{
		if (!CEFDebugAvailable()) {
			return nlohmann::json::array();
		}

		//if (Settings::common.extendedLogging)
		//{
		spdlog::trace("Fetching available Steam CEF tabs");
		//}

		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				return nlohmann::json::parse(res->body);
			}
		}

		return nlohmann::json::array();
	}

	nlohmann::basic_json<> InjectJs(std::string_view tab_name, std::string_view debug_url, std::wstring_view js, uint16_t port)
	{
#ifdef _WIN32
		INT rc;
		WSADATA wsaData;

		rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (rc) {
			printf("WSAStartup Failed.\n");
			return nullptr;
		}
#endif

		std::shared_ptr<easywsclient::WebSocket> ws{
			easywsclient::WebSocket::from_url(debug_url.data())
		};
		if (ws)
		{
			nlohmann::json res = nullptr;
			try
			{
				auto json_payload = nlohmann::json{
							{"id", internal::msg_id++},
							{"method", "Runtime.evaluate"},
							{"params", {
								{"userGesture", true},
								{"expression", std::wstring{js.data()}}
								//{"expression", js}
							}}
				};
				auto payload_string = json_payload.dump();

				spdlog::debug("Injecting JS into tab: {}, {}; JS: {}", tab_name, debug_url, payload_string);

				ws->send(payload_string);
				bool exit = false;
				while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
					ws->poll();
					ws->dispatch([&ws, &res, &exit](const std::string& message) {
						const auto msg = nlohmann::json::parse(message);
					try
					{
						if (msg.at("result").at("result").at("type").get<std::string>() != "undefined") {
							res = msg.at("result").at("result").at("value");
						}
					}
					catch (...) {
						spdlog::error("CEFInject: Error parsing injection-result value: {}", message);
						res = msg;
					}
					exit = true;
						});
					if (exit) {
						ws->close();
						return res;
					}
				}
			} catch (...) {
				spdlog::error(
					"CEFInject: Error injecting JS into tab: {}, {}",
					std::string(tab_name.data()),
					std::string(debug_url.data()));
			}
#ifdef _WIN32
			WSACleanup();
#endif
			return res;
		}
#ifdef _WIN32
		WSACleanup();
#endif
		return nullptr;
	}

	nlohmann::basic_json<> InjectJsByName(std::wstring_view tabname, std::wstring_view js, uint16_t port)
	{
		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				const auto json = nlohmann::json::parse(res->body);
				for (const auto& tab : json) {
					if (tab["title"].get<std::wstring>().find(tabname) != std::string::npos) {
						return InjectJs(tab["title"].get<std::string>(), tab["webSocketDebuggerUrl"].get<std::string>(), js, port);
					}
				}
			}
		}
		return nullptr;
	}


	bool SteamTweaks::injectGlosSITweaks(std::string_view tab_name, uint16_t port)
	{
		if (tab_name.empty()) {
			for (auto ts = AvailableTabNames(); const auto & tn : ts)
			{
				// meh!
				injectGlosSITweaks(util::string::to_string(tn), port);
			}
			return true;
		}
		return injectGlosSITweaks(Tab_Info{ std::string(tab_name) }, port);
	}

	bool SteamTweaks::injectGlosSITweaks(const Tab_Info& info, uint16_t port)
	{
		if (!CEFDebugAvailable()) {
			return false;
		}

		if (glossi_tweaks_js_.empty())
		{
			if (!readGlosSITweaksJs()) {
				return false;
			}
		}

		const auto find_tab = (
			[&info]() -> std::function<nlohmann::json(const nlohmann::json::array_t& tabList)>
			{
				if (!info.name.empty())
				{
					return [&info](const nlohmann::json::array_t& tabList)
					{
						for (const auto& tab : tabList) {
							if (tab["title"].get<std::string>().find(info.name.data()) != std::string::npos) {
								return tab;
							}
						}
						return nlohmann::json{};
					};
				}
		if (!info.id.empty())
		{
			return [&info](const nlohmann::json::array_t& tabList)
			{
				for (const auto& tab : tabList) {
					if (tab["id"].get<std::string>() == info.id) {
						return tab;
					}
				}
				return nlohmann::json{};
			};

		}
		if (!info.webSocketDebuggerUrl.empty())
		{
			return [&info](const nlohmann::json::array_t& tabList)
			{
				for (const auto& tab : tabList) {
					if (tab["webSocketDebuggerUrl"].get<std::string>() == info.webSocketDebuggerUrl) {
						return tab;
					}
				}
				return nlohmann::json{};
			};

		}
		return nullptr;
			}
		)();
		if (find_tab == nullptr) {
			return false;
		}
		const auto tabs = AvailableTabs(port);
		const auto tab = find_tab(tabs);
		if (tab.empty()) {
			return false;
		}

		const auto res = InjectJs(tab["title"].get<std::string>(), tab["webSocketDebuggerUrl"].get<std::string>(), glossi_tweaks_js_, port);
		if (res.is_boolean() && res.get<bool>()) {
			glossi_tweaks_injected_map_[tab["id"].get<std::string>()] = true;
			spdlog::trace("CEFInject: GlosSITweaks injected into tab: {}", tab["title"].get<std::string>());
			return true;
		}
		return false;
	}

	bool SteamTweaks::uninstallTweaks(bool force)
	{
		if (!CEFDebugAvailable()) {
			return false;
		}

		auto_inject_ = false;
		if (auto_inject_future_.valid()) {
			auto_inject_future_.wait();
		}

		if (glossi_tweaks_injected_map_.empty() && !force) {
			return false;
		}

		std::vector<std::future<void>> futures;
		for (auto ts = AvailableTabs(); auto & tab : ts) {
			futures.push_back(std::async(std::launch::async, [this, &tab]()
				{
					InjectJs(tab["title"].get<std::string>(), tab["webSocketDebuggerUrl"].get<std::string>(), uninstall_glossi_tweaks_js_);
				}));
		}

		for (auto& f : futures)
		{
			if (f.valid()){
				f.wait();
			}
		}
		glossi_tweaks_injected_map_.clear();
		return true;
	}

	void SteamTweaks::update(float elapsed_time)
	{
		if (!auto_inject_) {
			return;
		}
		using namespace std::chrono_literals;
		if (auto_inject_future_.valid() && auto_inject_future_.wait_for(0ms) != std::future_status::ready) {
			time_since_last_update_ = 0.0f;
			return;
		}

		time_since_last_update_ += elapsed_time;
		if (time_since_last_update_ < update_interval_) {
			return;
		}
		time_since_last_update_ = 0.0f;

		spdlog::trace("CEFInject: Starting auto inject GlosSITweaks");
		auto_inject_future_ = std::async(std::launch::async, [this]() {

			if (js_tweaks_cache_.empty()) [[unlikely]] {
				readAvailableTweaks();
			}

				if (glossi_tweaks_js_.empty()) [[unlikely]]
				{
					if (!readGlosSITweaksJs()) {
						return;
					}
				}

			auto futures = std::vector<std::future<void>>{};
			auto tabs = AvailableTabs();
			for (auto& tab : tabs) {
				if (glossi_tweaks_injected_map_.contains(tab["id"].get<std::string>())) {
					continue;
				}
				glossi_tweaks_injected_map_[tab["id"].get<std::string>()] = true;

				 futures.push_back(std::async([this, &tab]()
					{
						InjectJs(tab["title"].get<std::string>(), tab["webSocketDebuggerUrl"].get<std::string>(), glossi_tweaks_js_);

						for (auto& [path, js] : js_tweaks_cache_) {
							const auto dir_name = path.parent_path().filename();

							if (path_tab_map_.contains(dir_name.wstring())) {
								if (tab["title"].get<std::string>().find(path_tab_map_.at(dir_name.wstring())) != std::string::npos) {
									InjectJs(tab["title"].get<std::string>(), tab["webSocketDebuggerUrl"].get<std::string>(), js);
								}
							}
						}
					}));
			}
			for (auto& f : futures)
			{
				if (f.valid()) {
					f.wait();
				}
			}
			spdlog::trace("CEFInject: Auto Inject thread done");
			});
	}

	bool SteamTweaks::isAutoInject() const
	{
		return auto_inject_;
	}

	void SteamTweaks::setAutoInject(const bool auto_inject)
	{
		auto_inject_ = auto_inject;
	}

	bool SteamTweaks::readGlosSITweaksJs()
	{
		if (glossi_tweaks_js_.empty())
		{
			spdlog::trace("CEFInject: Loadings GlosSITweaks.js");

			auto glossi_path = util::path::getGlosSIDir();
			glossi_path /= steam_tweaks_path_;
			glossi_path /= "GlosSITweaks.js";

			if (!std::filesystem::exists(glossi_path))
			{
				spdlog::error("CEFInject: GlosSITweaks.js not found");
				return false;
			}

			std::wifstream js_file(glossi_path);
			glossi_tweaks_js_ = { (std::istreambuf_iterator<wchar_t>(js_file)),
				std::istreambuf_iterator<wchar_t>() };
			if (glossi_tweaks_js_.empty()) {
				spdlog::error("CEFInject: GlosSITweaks.js empty?");
				return false;
			}
			js_file.close();
		}
		return true;
	}

	void SteamTweaks::readAvailableTweaks(bool builtin)
	{
		auto tweaks_path = builtin ? util::path::getGlosSIDir() : util::path::getDataDirPath();
		spdlog::log(
			builtin ? spdlog::level::trace : spdlog::level::debug,
			"CEFInject: Loading {} {} {}",
			builtin ? "builtin" : "user",
			"tweaks from",
			tweaks_path.string()
		);
		tweaks_path /= steam_tweaks_path_;
		if (!std::filesystem::exists(tweaks_path))
		{
			return;
		}

		auto find_tweak_files = [this](std::wstring_view path, auto&& recurse) -> void
		{
			for (const auto& dir_file : std::filesystem::directory_iterator(path))
			{
				if (std::filesystem::is_directory(dir_file))
				{
					recurse(dir_file.path().wstring(), recurse);
					continue;
				}
				if (std::filesystem::is_regular_file(dir_file) && dir_file.path().extension() == ".js")
				{
					if (dir_file.path().filename() == "GlosSITweaks.js") {
						continue;
					}
					std::wifstream js_file(dir_file.path());
					std::wstring tweaks_js = { (std::istreambuf_iterator<wchar_t>(js_file)),
						std::istreambuf_iterator<wchar_t>() };
					if (tweaks_js.empty()) {
						continue;
					}
					js_file.close();
					js_tweaks_cache_[dir_file.path().wstring()] = tweaks_js;
				}
			}
		};
		find_tweak_files(tweaks_path.wstring(), find_tweak_files);


	}
}
