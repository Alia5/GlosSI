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

namespace CEFInject
{
	namespace internal
	{
		httplib::Client GetHttpClient(uint16_t port)
		{
			httplib::Client cli("localhost", port);
			cli.set_connection_timeout(1000);
			return cli;
		}

		static uint32_t msg_id = 0;

	}
	bool CEFDebugAvailable(uint16_t port)
	{
		auto cli = internal::GetHttpClient(port);
		cli.set_connection_timeout(500);
		cli.set_read_timeout(500);
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

	nlohmann::json::array_t AvailableTabs(uint16_t port)
	{
		if (!CEFDebugAvailable()) {
			return nlohmann::json::array();
		}
		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				return nlohmann::json::parse(res->body);
			}
		}
		return nlohmann::json::array();
	}

	nlohmann::json InjectJs(std::string_view debug_url, std::wstring_view js, uint16_t port)
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
			ws->send(payload_string);
			nlohmann::json res = nullptr;
			bool exit = false;
			while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
				ws->poll();
				ws->dispatch([&ws, &res, &exit](const std::string& message) {
					const auto msg = nlohmann::json::parse(message);
					try
					{
						res = msg.at("result").at("result").at("value");
					} catch (...){
						//
					}
					try
					{
						if (res.is_null() && msg.at("result").at("result").at("type").get<std::string>() != "undefined") {
							res = nlohmann::json::parse(message);
						}
					} catch (...) {
						res = nlohmann::json::parse(message);
					}
				exit = true;
					});
				if (exit) {
					ws->close();
					return res;
				}
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

	nlohmann::json InjectJsByName(std::wstring_view tabname, std::wstring_view js, uint16_t port)
	{
		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				const auto json = nlohmann::json::parse(res->body);
				for (const auto& tab : json) {
					if (tab["title"].get<std::wstring>().starts_with(tabname)) {
						return InjectJs(tab["webSocketDebuggerUrl"].get<std::string>(), js, port);
					}
				}
			}
		}
		return nullptr;
	}


	bool SteamTweaks::injectGlosSITweaks(std::string_view tab_name, uint16_t port)
	{
		if (tab_name.empty()) {
			for (const auto& tn : AvailableTabNames())
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

		auto glossi_path = util::path::getGlosSIDir();
		glossi_path /= steam_tweaks_path_;
		glossi_path /= "GlosSITweaks.js";

		if (!std::filesystem::exists(glossi_path))
		{
			return false;
		}

		std::wifstream js_file(glossi_path);
		std::wstring glossitweaks_js{ (std::istreambuf_iterator<wchar_t>(js_file)),
			std::istreambuf_iterator<wchar_t>() };
		if (glossitweaks_js.empty()) {
			return false;
		}

		const auto find_tab = (
			[&info]() -> std::function<nlohmann::json(const nlohmann::json::array_t& tabList)>
			{
				if (!info.name.empty())
				{
					return [&info](const nlohmann::json::array_t& tabList)
					{
						for (const auto& tab : tabList) {
							if (tab["title"].get<std::string>().starts_with(info.name.data())) {
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

		InjectJs(tab["webSocketDebuggerUrl"].get<std::string>(), glossitweaks_js, port);
		glossi_tweaks_injected_map_[tab["id"].get<std::string>()] = true;
		return true;
	}

	bool SteamTweaks::uninstallTweaks()
	{
		if (!CEFDebugAvailable()) {
			return false;
		}
		if (glossi_tweaks_injected_map_.empty()) {
			return false;
		}

		for (auto& tab : AvailableTabNames()) {
			InjectJsByName(tab, uninstall_glossi_tweaks_js_);
		}

		glossi_tweaks_injected_map_.clear();
		return true;
	}

}
