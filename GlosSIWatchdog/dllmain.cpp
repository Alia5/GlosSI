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

#include <httplib.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>

#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include "../version.hpp"
#include "../GlosSITarget/HidHide.h"

bool IsProcessRunning(DWORD pid)
{
	const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	if (process == nullptr)
		return false;
	const DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}

DWORD WINAPI watchdog(HMODULE hModule)
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
	logPath /= "GlosSIWatchdog.log";
	const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.wstring(), true);
	std::vector<spdlog::sink_ptr> sinks{ file_sink };
	auto logger = std::make_shared<spdlog::logger>("log", sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
	spdlog::set_default_logger(logger);

	spdlog::info("GlosSIWatchdog loaded");
	spdlog::info("Version: {}", version::VERSION_STR);

	auto glossi_hwnd = FindWindowA(nullptr, "GlosSITarget");
	if (!glossi_hwnd)
	{
		spdlog::error("Couldn't find GlosSITarget window. Exiting...");
		FreeLibraryAndExitThread(hModule, 1);
		return 1;
	}
	spdlog::debug("Found GlosSITarget window; Starting watch loop");

	httplib::Client http_client("http://localhost:8756");
	http_client.set_connection_timeout(1);
	auto http_res = http_client.Get("/launched-pids");
	std::vector<DWORD> pids = http_res.error() == httplib::Error::Success && http_res->status == 200 ? nlohmann::json::parse(http_res->body).get<std::vector<DWORD>>() : std::vector<DWORD>();
	while (glossi_hwnd)
	{
		http_client.set_connection_timeout(120);
		http_res = http_client.Get("/launched-pids");
		if (http_res.error() == httplib::Error::Success && http_res->status == 200)
		{
			pids = nlohmann::json::parse(http_res->body).get<std::vector<DWORD>>();
		} else {
			spdlog::error("Couldn't fetch launched PIDs: {}", (int)http_res.error());
		}

		glossi_hwnd = FindWindowA(nullptr, "GlosSITarget");

		Sleep(333);
	}
	spdlog::info("GlosSITarget was closed. Resetting HidHide state...");
	HidHide hidhide;
	hidhide.disableHidHide();
	// TODO: read settings; check if watchdog should close launched procs

	spdlog::info("Closing launched processes");


	for (const auto pid : pids)
	{
		if (IsProcessRunning(pid))
		{
			if (const auto proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid))
			{
				spdlog::debug("Terminating process: {}", pid);
				const auto terminate_res = TerminateProcess(proc, 0);
				if (!terminate_res)
				{
					spdlog::error("Failed to terminate process: {}", pid);
				}
				CloseHandle(proc);
			}
		}
	}

	// \
	
	spdlog::info("Unloading Watchdog...");
	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)watchdog, hModule, 0, nullptr));
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
	}
	return TRUE;

    return 0;
}
