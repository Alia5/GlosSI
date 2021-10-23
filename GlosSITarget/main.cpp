/*
Copyright 2021 Peter Repukat - FlatspotSoftware

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
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include "SteamTarget.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "OverlayLogSink.h"
#include "Settings.h"


#ifdef _WIN32
#ifdef CONSOLE
int main(int argc, char* argv[])
#else
int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
#endif
#else
int main(int argc, char* argv[])
#endif
{
    const auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
#ifdef _WIN32
    const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("./glossitarget.log", true);
#else
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("/tmp/glossitarget.log", true);
#endif
    file_sink->set_level(spdlog::level::trace);

    const auto overlay_sink = std::make_shared<spdlog::sinks::overlay_sink_mt>();
#ifdef _DEBUG
    overlay_sink->set_level(spdlog::level::debug); // TODO: make configurable
#else
    overlay_sink->set_level(spdlog::level::info);
#endif

    std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink, overlay_sink};
    auto logger = std::make_shared<spdlog::logger>("log", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(logger);
#ifdef _WIN32
    Settings::Parse(__argv[1]);
    SteamTarget target(__argc, __argv);
#else
    Settings::Parse(argv[1]);
    SteamTarget target(argc, argv);
#endif
    const auto exit = target.run();
    spdlog::shutdown();
    return exit;
}