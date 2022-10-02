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

#define NOMINMAX
#include <Windows.h>

#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "../version.hpp"
#include "../GlosSITarget/HidHide.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	auto configDirPath = std::filesystem::temp_directory_path()
		.parent_path()
		.parent_path()
		.parent_path();

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
		return 1;
	}
	spdlog::debug("Found GlosSITarget window; Starting watch loop");

    while (glossi_hwnd)
    {
    	glossi_hwnd = FindWindowA(nullptr, "GlosSITarget");
		Sleep(1337);
    }
	spdlog::info("GlosSITarget was closed. Cleaning up...");
	HidHide hidhide;
	hidhide.disableHidHide();

    return 0;
}
