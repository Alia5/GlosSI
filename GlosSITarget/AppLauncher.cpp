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
#include "AppLauncher.h"

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <ShObjIdl.h>
#include <atlbase.h>
#include <tlhelp32.h>
#include <Propsys.h>
#include <propkey.h>

#pragma comment(lib, "Shell32.lib")
#endif
#include "Settings.h"

#include <regex>

AppLauncher::AppLauncher(
    std::vector<HWND>& process_hwnds,
    std::function<void()> shutdown) : process_hwnds_(process_hwnds), shutdown_(std::move(shutdown))
{
#ifdef _WIN32
    UnPatchValveHooks();
#endif
};

void AppLauncher::launchApp(const std::wstring& path, const std::wstring& args)
{
#ifdef _WIN32
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
#endif
}

void AppLauncher::update()
{
    if (process_check_clock_.getElapsedTime().asMilliseconds() > 250) {
#ifdef _WIN32
        if (!pids_.empty() && pids_[0] > 0) {
            if (Settings::launch.waitForChildProcs) {
                getChildPids(pids_[0]);
            }
            if (!IsProcessRunning(pids_[0])) {
                spdlog::info("Launched App with PID \"{}\" died", pids_[0]);
                if (Settings::launch.closeOnExit && !Settings::launch.waitForChildProcs) {
                    spdlog::info("Configured to close on exit. Shutting down...");
                    shutdown_();
                }
                pids_[0] = 0;
            }
        }
        if (Settings::launch.waitForChildProcs) {
            std::erase_if(pids_, [](auto pid) {
                const auto running = IsProcessRunning(pid);
                if (!running)
                    spdlog::info("Child process with PID \"{}\" died", pid);
                return !running;
            });
            if (Settings::launch.closeOnExit && pids_.empty()) {
                spdlog::info("Configured to close on all children exit. Shutting down...");
                shutdown_();
            }
        }
        getProcessHwnds();
#endif
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
                pids_.push_back(pe.th32ProcessID);
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
            propStore->GetValue(PKEY_AppUserModel_ID, &prop);
            if (prop.bstrVal != nullptr && std::wstring(prop.bstrVal) == launched_uwp_path_) {
                process_hwnds_.push_back(curr_wnd);
            }
        } while (curr_wnd != nullptr);
    }
}

#endif

#ifdef _WIN32
void AppLauncher::UnPatchValveHooks()
{
    spdlog::debug("Unpatching Valve CreateProcess hook...");
    // need to load addresses that way.. Otherwise we may land before some jumps...
    auto kernel32dll = GetModuleHandle(L"kernel32.dll");
    if (kernel32dll) {
        BYTE* address = reinterpret_cast<BYTE*>(GetProcAddress(kernel32dll, "CreateProcessW"));
        if (address) {
            DWORD dw_old_protect, dw_bkup;
            const auto len = CREATE_PROC_ORIG_BYTES.size();
            VirtualProtect(address, len, PAGE_EXECUTE_READWRITE, &dw_old_protect); //Change permissions of memory..
            for (DWORD i = 0; i < len; i++)                                        //unpatch Valve's hook
            {
                *(address + i) = CREATE_PROC_ORIG_BYTES[i];
            }
            VirtualProtect(address, len, dw_old_protect, &dw_bkup); //Revert permission change...
            spdlog::trace("Unpatched CreateProcessW");
        }
        else {
            spdlog::error("failed to unpatch CreateProcessW");
        }
    }
    else {
        spdlog::error("kernel32.dll not found... sure...");
    }
}

void AppLauncher::launchWin32App(const std::wstring& path, const std::wstring& args)
{
    const auto native_seps_path = std::regex_replace(path, std::wregex(L"(\\/|\\\\)"), L"\\");
    //std::wstring launch_dir;
    //std::wsmatch m;
    //if (!std::regex_search(native_seps_path, m, std::wregex(L"(.*?\\\\)*"))) {
    //    spdlog::warn("Couldn't detect launch application directory"); // Shouldn't ever happen...
    //} else {
    //    launch_dir = m[0];
    //}
    std::wstring args_cpy(args);
    spdlog::debug(L"Launching Win32App app \"{}\"; args \"{}\"", native_seps_path, args_cpy);
    if (CreateProcessW(native_seps_path.data(),
                       args_cpy.data(),
                       nullptr,
                       nullptr,
                       TRUE,
                       0,
                       nullptr,
                       nullptr, //launch_dir.empty() ? nullptr : launch_dir.data(),
                       &info,
                       &process_info)) {
        //spdlog::info(L"Started Program: \"{}\" in directory: \"{}\"", native_seps_path, launch_dir);
        spdlog::info(L"Started Program: \"{}\"", native_seps_path);
        pids_.push_back(process_info.dwProcessId);
    }
    else {
        //spdlog::error(L"Couldn't start program: \"{}\" in directory: \"{}\"", native_seps_path, launch_dir);
        spdlog::error(L"Couldn't start program: \"{}\"", native_seps_path);
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
                spdlog::info(L"Launched UWP Package \"{}\"", package_full_name);
            }
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

    if (execute_info.hProcess != nullptr) {
        if (const auto pid = GetProcessId(execute_info.hProcess); pid > 0) {
            pids_.push_back(pid);
            return;
        }
    }
    spdlog::warn("Couldn't get PID of launched URL process");
}
#endif
