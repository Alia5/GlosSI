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

#include <future>
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
	nlohmann::basic_json<> AvailableTabs(uint16_t port = internal::port_);
	nlohmann::basic_json<> InjectJs(std::string_view tab_name, std::string_view debug_url, std::wstring_view js, uint16_t port = internal::port_);
	nlohmann::basic_json<> InjectJsByName(std::wstring_view tabname, std::wstring_view js, uint16_t port = internal::port_);

	class WSAStartupWrap
	{
		public:
		explicit WSAStartupWrap();
			~WSAStartupWrap();
		private:
			bool wsa_startup_succeeded_ = false;
	};

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
		bool uninstallTweaks(bool force = false);

		void update(float elapsed_time);

		[[nodiscard]] bool isAutoInject() const;
		void setAutoInject(const bool auto_inject);
	private:
		bool readGlosSITweaksJs();
		void readAvailableTweaks(bool builtin = true);
		bool auto_inject_ = false;

		static constexpr float update_interval_ = 30.f;
		float time_since_last_update_ = update_interval_;
		using tab_id = std::string;
		std::map<tab_id, bool> glossi_tweaks_injected_map_;

		std::future<void> auto_inject_future_;

		std::wstring glossi_tweaks_js_;

		std::map<std::filesystem::path, std::wstring> js_tweaks_cache_;

		using path_name = std::wstring;
		using tab_name = std::string;
		static inline const std::map<path_name, tab_name> path_tab_map_ = {
			{L"SharedContext", "Steam Shared Context"},
			{L"Overlay", "HOW TF GET OVERLAY TAB NAME?"}, // TODO: Figure out how to get the overlay tab name
            {L"GamepadUI", "Steam Big Picture Mode"},
		};

		static constexpr std::string_view steam_shared_ctx_tab_name_ = "Steam Shared Context";
		static constexpr std::string_view steam_tweaks_path_ = "SteamTweaks";
		static constexpr std::wstring_view uninstall_glossi_tweaks_js_ = LR"(
				(() => {
					return window.GlosSITweaks?.GlosSI?.uninstall();
				})();
			)";
	};

	namespace internal {
		static WSAStartupWrap wsa_startup_wrap{};
	}

}