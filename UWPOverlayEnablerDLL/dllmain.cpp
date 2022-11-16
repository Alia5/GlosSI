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

/*

There are two (known to me, at time of writing) ways to get a working overlay for UWP Apps
1. Create an accessibility app
	Set UIAcces in manifest to true
	This however requires that the application is digitally signed
	and is run from a trusted directory (Program Files; System32)
	At this point UWP overlays are not a technical issue anymore, but a monetary
	I have no interest in spending ~100 bucks a year just to provide this functionality to users.

	You could also self-sign the application, but installing a trusted root CA is a security risk

2.  Use undocumented SetWindowBand function
	This function however is not freely callable from every process.
	Even when injected into explorer.exe, it doesn't seem to work when just calling normally...
	So let's hook the original function, and try to do "the magic" then
	This seemingly works ¯\_(ツ)_/¯
	
	"The magic":
		Hook SetWindowBand
		Once called, find GlosSI Window
		Set GlosSI Window to ZBID_SYSTEM_TOOLS (Doesn't seem to require any special stuff)
		Self-Eject
		
		**PROFIT!**

*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>

#define SUBHOOK_STATIC
#include <atomic>
#include <filesystem>
#include <subhook.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <chrono>

enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};

typedef BOOL(WINAPI* fSetWindowBand)(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand);


subhook::Hook SetWindowBandHook;
fSetWindowBand SetWindowBand;

std::atomic<bool> allow_exit = false;

std::atomic<ZBID> to_set_window_band = ZBID_SYSTEM_TOOLS;

std::chrono::time_point<std::chrono::system_clock> start_time;

static constexpr int hook_timeout_seconds_ = 60;

BOOL WINAPI SetGlosSIWindowBand(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand)
{
	subhook::ScopedHookRemove remove(&SetWindowBandHook);
	const auto glossi_hwnd = FindWindowA(nullptr, "GlosSITarget");
	if (glossi_hwnd)
	{
		spdlog::info("Found GlosSI Window");
		// Most window bands don't really seem to work.
		// However, notification and system_tools does!
		// use system tools, as that allows the steam overlay to be interacted with
		// without UWP apps minimizing
		auto success = SetWindowBand(glossi_hwnd, nullptr, to_set_window_band);
		allow_exit = true;
		spdlog::info("Set GlosSI Window Band to {}; success: {}", static_cast<int>(to_set_window_band), success);
	} else if (allow_exit == false) {
		const auto time = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(time - start_time).count() > hook_timeout_seconds_)
		{
			spdlog::info("GlosSI Window not found after {} seconds, exiting", hook_timeout_seconds_);
			allow_exit = true;
		}
	}
	return SetWindowBand(hWnd, hwndInsertAfter, dwBand);
}

DWORD WINAPI WaitThread(HMODULE hModule)
{
	while (!allow_exit)
	{
		Sleep(10);
	}
	if (SetWindowBandHook.IsInstalled())
	{
		spdlog::debug("Uninstalling SetWindowBand hook");
		SetWindowBandHook.Remove();
	}
	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		wchar_t* localAppDataFolder;
		std::filesystem::path configDirPath;
		if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) != S_OK) {
			configDirPath = std::filesystem::temp_directory_path().parent_path().parent_path().parent_path();
		}
		else {
			configDirPath = std::filesystem::path(localAppDataFolder).parent_path();
		}

		configDirPath /= "Roaming";
		configDirPath /= "GlosSI";
		if (!std::filesystem::exists(configDirPath))
			std::filesystem::create_directories(configDirPath);



		auto logPath = configDirPath;
		logPath /= "UWPOverlayEnabler.log";
		const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
		std::vector<spdlog::sink_ptr> sinks{ file_sink };
		auto logger = std::make_shared<spdlog::logger>("log", sinks.begin(), sinks.end());
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::trace);
		spdlog::set_default_logger(logger);

		spdlog::info("UWPOverlayEnabler loaded");

		auto configPath = configDirPath;
		configPath /= "UWPOverlayEnabler.cfg";
		if (std::filesystem::exists(configPath))
		{
			std::ifstream config(configPath);
			std::string line;
			while (std::getline(config, line))
			{
				// github copilot, lol
				// i take it!
				if (line == "ZBID_DEFAULT")
					to_set_window_band = ZBID_DEFAULT;
				else if (line == "ZBID_DESKTOP")
					to_set_window_band = ZBID_DESKTOP;
				else if (line == "ZBID_UIACCESS")
					to_set_window_band = ZBID_UIACCESS;
				else if (line == "ZBID_IMMERSIVE_IHM")
					to_set_window_band = ZBID_IMMERSIVE_IHM;
				else if (line == "ZBID_IMMERSIVE_NOTIFICATION")
					to_set_window_band = ZBID_IMMERSIVE_NOTIFICATION;
				else if (line == "ZBID_IMMERSIVE_APPCHROME")
					to_set_window_band = ZBID_IMMERSIVE_APPCHROME;
				else if (line == "ZBID_IMMERSIVE_MOGO")
					to_set_window_band = ZBID_IMMERSIVE_MOGO;
				else if (line == "ZBID_IMMERSIVE_EDGY")
					to_set_window_band = ZBID_IMMERSIVE_EDGY;
				else if (line == "ZBID_IMMERSIVE_INACTIVEMOBODY")
					to_set_window_band = ZBID_IMMERSIVE_INACTIVEMOBODY;
				else if (line == "ZBID_IMMERSIVE_INACTIVEDOCK")
					to_set_window_band = ZBID_IMMERSIVE_INACTIVEDOCK;
				else if (line == "ZBID_IMMERSIVE_ACTIVEMOBODY")
					to_set_window_band = ZBID_IMMERSIVE_ACTIVEMOBODY;
				else if (line == "ZBID_IMMERSIVE_ACTIVEDOCK")
					to_set_window_band = ZBID_IMMERSIVE_ACTIVEDOCK;
				else if (line == "ZBID_IMMERSIVE_BACKGROUND")
					to_set_window_band = ZBID_IMMERSIVE_BACKGROUND;
				else if (line == "ZBID_IMMERSIVE_SEARCH")
					to_set_window_band = ZBID_IMMERSIVE_SEARCH;
				else if (line == "ZBID_GENUINE_WINDOWS")
					to_set_window_band = ZBID_GENUINE_WINDOWS;
				else if (line == "ZBID_IMMERSIVE_RESTRICTED")
					to_set_window_band = ZBID_IMMERSIVE_RESTRICTED;
				else if (line == "ZBID_SYSTEM_TOOLS")
					to_set_window_band = ZBID_SYSTEM_TOOLS;
				else if (line == "ZBID_LOCK")
					to_set_window_band = ZBID_LOCK;
				else if (line == "ZBID_ABOVELOCK_UX")
					to_set_window_band = ZBID_ABOVELOCK_UX;

			}
			spdlog::info("Read window band from config: {}", static_cast<int>(to_set_window_band));
		}

		const auto hpath = LoadLibrary(L"user32.dll");
		if (hpath)
		{
			start_time = std::chrono::system_clock::now();
			spdlog::debug("Loaded user32.dll");
			spdlog::debug("Installing SetWindowBand hook");
			SetWindowBand = reinterpret_cast<fSetWindowBand>(GetProcAddress(hpath, "SetWindowBand"));
			SetWindowBandHook.Install(GetProcAddress(hpath, "SetWindowBand"), &SetGlosSIWindowBand, subhook::HookFlags::HookFlag64BitOffset);
			spdlog::debug("Creating wait thread");
			CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)WaitThread, hModule, 0, nullptr));
		} else
		{
			spdlog::error("Loaded user32.dll");
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		spdlog::info("unloading UWPOverlayEnabler");
		if (SetWindowBandHook.IsInstalled())
		{
			spdlog::debug("Uninstalling SetWindowBand hook");
			SetWindowBandHook.Remove();
		}
	}
    return TRUE;
}

