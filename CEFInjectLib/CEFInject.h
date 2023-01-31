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

#include <httplib.h>
#include <nlohmann/json.hpp>


namespace CEFInject
{
	namespace internal {
		httplib::Client GetHttpClient(uint16_t port);
		static inline uint16_t port_ = 8080;
	}
	inline void setPort(uint16_t port)
	{
		internal::port_ = port;
	}
	bool CEFDebugAvailable(uint16_t port = internal::port_);
	std::vector<std::wstring> AvailableTabNames(uint16_t port = internal::port_);
	nlohmann::json::array_t AvailableTabs(uint16_t port = internal::port_);
	nlohmann::json InjectJs(std::string_view debug_url, std::wstring_view js, uint16_t port = internal::port_);
	nlohmann::json InjectJsByName(std::wstring_view tabname, std::wstring_view js, uint16_t port = internal::port_);

	class SteamTweaks
	{
	public:
		SteamTweaks() = default;

		struct Tab_Info
		{
			std::string name;
			std::string id;
			std::string webSocketDebuggerUrl;
		};
		bool injectGlosSITweaks(std::string_view tab_name, uint16_t port = internal::port_);
		bool injectGlosSITweaks(const Tab_Info& info, uint16_t port = internal::port_);
	public:
		bool uninstallTweaks(bool force = false);
		
		// TODO: Provide API to auto-inject

		// TODO: build system to auto inject "user plugins"

	private:
		using tab_id = std::string;
		std::map<tab_id, bool> glossi_tweaks_injected_map_;

		static constexpr std::string_view steam_tweaks_path_ = "SteamTweaks";
		static constexpr std::wstring_view uninstall_glossi_tweaks_js_ = LR"(
				(() => {
					return window.GlosSITweaks?.GlosSI?.uninstall();
				})();
			)";
	};

}