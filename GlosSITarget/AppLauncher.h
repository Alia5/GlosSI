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

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <functional>
#include <mutex>
#include <array>
#include <string>
#include <unordered_set>
#include <SFML/System/Clock.hpp>

class AppLauncher {
  public:
    explicit AppLauncher(
        std::vector<HWND>& process_hwnds,
        std::function<void()> shutdown = []() {});

    void launchApp(const std::wstring& path, const std::wstring& args = L"");
    void update();
    void close();

    std::vector<DWORD> launchedPids();
    void addPids(const std::vector<DWORD>& pids);
    
  private:
    std::function<void()> shutdown_;
    sf::Clock process_check_clock_;
    std::mutex pid_mutex_;

#ifdef _WIN32
    static bool IsProcessRunning(DWORD pid);
    void getChildPids(DWORD parent_pid);
    void getProcessHwnds();
    std::vector<HWND>& process_hwnds_;

    bool has_extra_launchers_ = false;
    bool launcher_has_launched_game_ = false;
    bool findLauncherPids();


    std::wstring launched_uwp_path_;
    
    static void UnPatchValveHooks();
    void launchWin32App(const std::wstring& path, const std::wstring& args = L"", bool watchdog = false);
    void launchUWPApp(LPCWSTR package_full_name, const std::wstring& args = L"");
    void launchURL(const std::wstring& url, const std::wstring& args = L"", const std::wstring& verb = L"open");
    STARTUPINFO info{sizeof(info)};
    PROCESS_INFORMATION process_info{};
    std::vector<DWORD> pids_;
#endif
};
