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
#include "AppLauncher.h"

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <ShObjIdl.h>
#include <atlbase.h>
#include <tlhelp32.h>
#include <Propsys.h>
#include <propkey.h>
#include <shellapi.h>
#include <winerror.h>

#pragma comment(lib, "Shell32.lib")
#endif
#include "..\common\Settings.h"

#include <regex>

#include "Overlay.h"
#include "../common/UnhookUtil.h"
#include "../common/util.h"

AppLauncher::AppLauncher(
    std::vector<HWND>& process_hwnds,
    std::function<void()> shutdown) : shutdown_(std::move(shutdown)), process_hwnds_(process_hwnds)
{
#ifdef _WIN32
    spdlog::debug("Unpatching Valve CreateProcess Hooks");
    UnPatchValveHooks();
    if (Settings::launch.launch) {
        spdlog::debug("App launch requested");
    }
#endif
};

void AppLauncher::launchApp(const std::wstring& path, const std::wstring& args)
{
#ifdef _WIN32

    if (!Settings::launch.launcherProcesses.empty()) {
        has_extra_launchers_ = true;
        spdlog::debug("Has extra launchers");
    }

    if (Settings::launch.isUWP) {
        spdlog::info("LaunchApp is UWP, launching...");
        launched_uwp_path_ = path;
        launchUWPApp(path.data(), args);
    }
    else if (path.find(L"://") != std::wstring::npos) {
        spdlog::info("LaunchApp is URL, launching...");
        launchURL(path, args);
    }
    else {
        spdlog::info("LaunchApp is Win32, launching...");
        launchWin32App(path, args);
    }
    Overlay::AddOverlayElem([this](bool has_focus, ImGuiID dockspace_id) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Launched Processes")) {
            ImGui::BeginChild("Inner##LaunchedProcs", {0.f, ImGui::GetItemRectSize().y - 64}, true);
            std::ranges::for_each(pids_, [](DWORD pid) {
                ImGui::Text("%s | %d", util::string::to_string(util::win::process::GetProcName(pid)).c_str(), pid);
                ImGui::SameLine();
                if (ImGui::Button((" Kill ##" + std::to_string(pid)).c_str())) {
                    util::win::process::KillProcess(pid);
                }
            });
            ImGui::EndChild();
        }
        ImGui::End();
    });
#endif
}

void AppLauncher::update()
{
    if (process_check_clock_.getElapsedTime().asMilliseconds() > 250) {
        pid_mutex_.lock();
#ifdef _WIN32
        if (has_extra_launchers_ && pids_.empty()) {
            findLauncherPids();
        }
        if (!pids_.empty() && pids_[0] > 0) {
            if (Settings::launch.waitForChildProcs) {
                getChildPids(pids_[0]);
            }
            if (!IsProcessRunning(pids_[0])) {
                spdlog::info(L"Launched App \"{}\" with PID \"{}\" died", util::win::process::GetProcName(pids_[0]), pids_[0]);
                if (Settings::launch.closeOnExit && !Settings::launch.waitForChildProcs && Settings::launch.launch) {
                    spdlog::info("Configured to close on exit. Shutting down...");
                    shutdown_();
                }
                pids_[0] = 0;
            }
        }
        if (Settings::launch.waitForChildProcs) {

            std::erase_if(pids_, [](auto pid) {
                if (pid == 0) {
                    return true;
                }
                const auto running = IsProcessRunning(pid);
                if (!running)
                    spdlog::trace(L"Child process \"{}\" with PID \"{}\" died", util::win::process::GetProcName(pid), pid);
                return !running;
            });

            auto filtered_pids = pids_ | std::ranges::views::filter([](DWORD pid) {
                                     return std::ranges::find(Settings::launch.launcherProcesses, util::win::process::GetProcName(pid)) == Settings::launch.launcherProcesses.end();
                                 });
            if (has_extra_launchers_ && !filtered_pids.empty()) {
                launcher_has_launched_game_ = true;
            }
            if (Settings::launch.closeOnExit && Settings::launch.launch) {
                if (has_extra_launchers_ && (Settings::launch.ignoreLauncher || Settings::launch.killLauncher)) {
                    if (launcher_has_launched_game_ && filtered_pids.empty()) {
                        spdlog::info("Configured to close on all children exit. Shutting down after game launched via EGS quit...");
                        shutdown_();
                    }
                }
                else {
                    if (pids_.empty()) {
                        spdlog::info("Configured to close on all children exit. Shutting down...");
                        shutdown_();
                    }
                }
            }
        }
        getProcessHwnds();
#endif
        pid_mutex_.unlock();
        process_check_clock_.restart();
    }
}

void AppLauncher::close()
{
#ifdef _WIN32
    if (process_info.dwProcessId > 0) {
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
#endif
}

std::vector<DWORD> AppLauncher::launchedPids()
{
    pid_mutex_.lock();
    std::vector<DWORD> res;
    res.reserve(pids_.size());
    if (!Settings::launch.killLauncher && Settings::launch.ignoreLauncher) {
        for (const auto& pid : pids_ | std::ranges::views::filter(
                                           [](DWORD pid) {
                                               return std::ranges::find(
                                                          Settings::launch.launcherProcesses,
                                                          util::win::process::GetProcName(pid)) == Settings::launch.launcherProcesses.end();
                                           })) {
            res.push_back(pid);
        }
    }
    else {
        std::ranges::copy(pids_.begin(), pids_.end(),
                          std::back_inserter(res));
    }
    pid_mutex_.unlock();
    return res;
}

void AppLauncher::addPids(const std::vector<DWORD>& pids)
{
    pid_mutex_.lock();
    for (const auto pid : pids) {
        if (pid > 0 && std::ranges::find(pids_, pid) == pids_.end()) {
            if (Settings::common.extendedLogging) {
                spdlog::debug("Added PID {} via API", pid);
            }
            pids_.push_back(pid);
        }
    }
    pid_mutex_.unlock();
}

#ifdef _WIN32
bool AppLauncher::IsProcessRunning(DWORD pid)
{
    const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (process == nullptr)
        return false;
    const DWORD ret = WaitForSingleObject(process, 0);
    CloseHandle(process);
    return ret == WAIT_TIMEOUT;
}

void AppLauncher::getChildPids(DWORD parent_pid)
{
    HANDLE hp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hp, &pe)) {
        do {
            if (pe.th32ParentProcessID == parent_pid) {
                if (std::ranges::find(pids_, pe.th32ProcessID) == pids_.end()) {
                    if (Settings::common.extendedLogging) {
                        spdlog::info(L"Found new child process \"{}\" with PID \"{}\"", util::win::process::GetProcName(pe.th32ProcessID), pe.th32ProcessID);
                    }
                    pids_.push_back(pe.th32ProcessID);
                    getChildPids(pe.th32ProcessID);
                }
            }
        } while (Process32Next(hp, &pe));
    }
    CloseHandle(hp);
}

void AppLauncher::getProcessHwnds()
{
    process_hwnds_.clear();
    HWND curr_wnd = nullptr;
    do {
        curr_wnd = FindWindowEx(nullptr, curr_wnd, nullptr, nullptr);
        DWORD check_pid = 0;
        GetWindowThreadProcessId(curr_wnd, &check_pid);
        if ((std::ranges::find_if(pids_, [check_pid](auto pid) {
                 return pid == check_pid;
             }) != pids_.end())) {
            process_hwnds_.push_back(curr_wnd);
        }
    } while (curr_wnd != nullptr);
    if (!launched_uwp_path_.empty()) {
        // UWP and ApplicationFrameHost Bullshit.
        // iterate all "ApplicationFrameWindow"; check the AppUserModelId (used for launching) and add on match.
        do {
            curr_wnd = FindWindowEx(nullptr, curr_wnd, L"ApplicationFrameWindow", nullptr);
            IPropertyStore* propStore;
            SHGetPropertyStoreForWindow(curr_wnd, IID_IPropertyStore, reinterpret_cast<void**>(&propStore));
            PROPVARIANT prop;
            if (propStore != nullptr) {
                propStore->GetValue(PKEY_AppUserModel_ID, &prop);
                if (prop.bstrVal != nullptr && std::wstring(prop.bstrVal) == launched_uwp_path_) {
                    process_hwnds_.push_back(curr_wnd);
                }   
            }
        } while (curr_wnd != nullptr);
    }
}

#endif

#ifdef _WIN32
bool AppLauncher::findLauncherPids()
{
    if (const auto pid = util::win::process::PidByName(L"EpicGamesLauncher.exe")) {
        spdlog::debug("Found EGS-Launcher running");
        pids_.push_back(pid);
        return true;
    }
    return false;
}

void AppLauncher::UnPatchValveHooks()
{
    // need to load addresses that way.. Otherwise we may land before some jumps...
    auto kernel32dll = GetModuleHandle(L"kernel32.dll");
    if (kernel32dll) {
        UnhookUtil::UnPatchHook("CreateProcessW", kernel32dll);
    }
    else {
        spdlog::error("kernel32.dll not found... sure...");
    }
}

void AppLauncher::launchWin32App(const std::wstring& path, const std::wstring& args, bool watchdog)
{
    const auto native_seps_path = std::regex_replace(path, std::wregex(L"(\\/|\\\\)"), L"\\");
    // std::wstring launch_dir;
    // std::wsmatch m;
    // if (!std::regex_search(native_seps_path, m, std::wregex(L"(.*?\\\\)*"))) {
    //     spdlog::warn("Couldn't detect launch application directory"); // Shouldn't ever happen...
    // } else {
    //     launch_dir = m[0];
    // }
    std::wstring args_cpy(
        args.empty()
        ? L""
            : ((native_seps_path.find(L" ") != std::wstring::npos
                    ? L"\"" + native_seps_path + L"\""
                    : native_seps_path) + L" " + args)
    );

    DWORD pid;

    spdlog::debug(L"Launching Win32App app \"{}\"; args \"{}\"", native_seps_path, args_cpy);
    if (CreateProcessW(native_seps_path.data(),
                       args_cpy.empty() ? nullptr : args_cpy.data(),
                       nullptr,
                       nullptr,
                       watchdog ? FALSE : TRUE,
                       watchdog ? DETACHED_PROCESS : 0,
                       nullptr,
                       nullptr, // launch_dir.empty() ? nullptr : launch_dir.data(),
                       &info,
                       &process_info)) {

        pid = process_info.dwProcessId;
    } else {
        DWORD error_code = GetLastError();

        if (error_code == ERROR_ELEVATION_REQUIRED) {
            spdlog::info("Elevated permissions required. Trying again with elevated permissions");

            SHELLEXECUTEINFOW shExecInfo = {0};
            shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
            shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
            shExecInfo.hwnd = NULL;
            shExecInfo.lpVerb = L"runas";
            shExecInfo.lpFile = native_seps_path.data();
            shExecInfo.lpParameters = args_cpy.empty() ? nullptr : args_cpy.data();
            shExecInfo.lpDirectory = nullptr; // launch_dir.empty() ? nullptr : launch_dir.data(),
            shExecInfo.nShow = SW_SHOW;
            shExecInfo.hInstApp = NULL;

            if (ShellExecuteExW(&shExecInfo)) {
                pid = GetProcessId(shExecInfo.hProcess);
                if (pid == 0u) {
                    spdlog::error(L"Couldn't get process id after starting program: \"{}\"; Error code {}", native_seps_path, GetLastError());
                }
                CloseHandle(shExecInfo.hProcess);
            } else {
                spdlog::error(L"Couldn't start program with elevated permissions: \"{}\"; Error code {}", native_seps_path, GetLastError());
                return;
            }
        } else {
            spdlog::error(L"Could't start program: \"{}\"; Error code: {}", native_seps_path, error_code);
            return;
        }
    }

    spdlog::info(L"Started Program: \"{}\"; PID: {}", native_seps_path, pid);

    if (!watchdog) {
        pid_mutex_.lock();
        pids_.push_back(pid);
        pid_mutex_.unlock();
    }
}

void AppLauncher::launchUWPApp(const LPCWSTR package_full_name, const std::wstring& args)
{
    spdlog::debug(L"Launching UWP app \"{}\"; args \"{}\"", package_full_name, args);
    HRESULT result = CoInitialize(nullptr);
    if (SUCCEEDED(result)) {

        CComPtr<IApplicationActivationManager> sp_app_activation_manager;
        // Initialize IApplicationActivationManager
        result = CoCreateInstance(
            CLSID_ApplicationActivationManager,
            nullptr,
            CLSCTX_LOCAL_SERVER,
            IID_IApplicationActivationManager,
            reinterpret_cast<LPVOID*>(&sp_app_activation_manager));

        if (SUCCEEDED(result)) {
            // This call ensures that the app is launched as the foreground window and sometimes may randomly fail...
            result = CoAllowSetForegroundWindow(sp_app_activation_manager, nullptr);
            if (!SUCCEEDED(result)) {
                spdlog::warn("CoAllowSetForegroundWindow failed. Code: {}", result);
            }
            pid_mutex_.lock();
            pids_.push_back(0);
            // Launch the app
            result = sp_app_activation_manager->ActivateApplication(
                package_full_name,
                args.empty() ? nullptr : args.data(),
                AO_NONE,
                &pids_[0]);
            if (!SUCCEEDED(result)) {
                spdlog::error("ActivateApplication failed: Code {}", result);
                pids_.clear();
            }
            else {
                spdlog::info(L"Launched UWP Package \"{}\"; PID: {}", package_full_name, pids_[0]);
            }
            pid_mutex_.unlock();
        }
        else {
            spdlog::error("CoCreateInstance failed: Code {}", result);
        }
        CoUninitialize();
    }
    else {
        spdlog::error("CoInitialize failed: Code {}", result);
    }
}

void AppLauncher::launchURL(const std::wstring& url, const std::wstring& args, const std::wstring& verb)
{
    HRESULT result = CoInitialize(nullptr);
    if (!SUCCEEDED(result)) {
        spdlog::error("CoInitialize failed: Code {}", result);
        return;
    }
    SHELLEXECUTEINFOW execute_info{};
    execute_info.cbSize = sizeof(SHELLEXECUTEINFOW);
    execute_info.fMask = SEE_MASK_NOASYNC; // because we exit process after ShellExecuteEx()
    execute_info.lpVerb = verb.c_str();
    execute_info.lpFile = url.c_str();
    execute_info.lpParameters = args.c_str();
    execute_info.nShow = SW_SHOWDEFAULT;

    if (!ShellExecuteExW(&execute_info)) {
        auto err = GetLastError();
        spdlog::error("Couldn't launch URL: ShellExecuteEx failed with error \"{}\"", err);
        CoUninitialize();
        if (err == ERROR_ACCESS_DENIED && verb == L"open") {
            spdlog::error("Error was access denied. Retry with elevated permissions...");
            launchURL(url, args, L"runas");
            return;
        }
    }
    CoUninitialize();

    if (url.find(L"epicgames.launcher") != std::wstring::npos) {
        has_extra_launchers_ = true;
    }
    if (execute_info.hProcess != nullptr) {
        if (const auto pid = GetProcessId(execute_info.hProcess); pid > 0) {
            pid_mutex_.lock();
            pids_.push_back(pid);
            spdlog::trace("Launched URL; PID: {}", pid);
            pid_mutex_.unlock();
            return;
        }
    }

    if (has_extra_launchers_) {
        spdlog::debug("Epic Games launch; Couldn't find egs launcher PID");
        pid_mutex_.lock();

        const auto pid = util::win::process::PidByName(L"EpicGamesLauncher.exe");
        if (!findLauncherPids()) {
            spdlog::debug("Did not find EGS-Launcher not running, retrying later...");
        }
        pid_mutex_.unlock();
        return;
    }
    spdlog::warn("Couldn't get PID of launched URL process");
}
#endif
