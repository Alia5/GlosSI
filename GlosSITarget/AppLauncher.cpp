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
#define COBJMACROS
#include <winternl.h>
#include <objbase.h>
#include <sddl.h>
#include <ShObjIdl.h>
#include <atlbase.h>
#include <tlhelp32.h>
#include <atlcomcli.h>
#endif
#include "Settings.h"

#include <regex>

AppLauncher::AppLauncher(std::function<void()> shutdown) : shutdown_(std::move(shutdown))
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
        launchUWPApp(path.data(), args);
    } else {
        spdlog::info("LaunchApp is Win32, launching...");
        launchWin32App(path, args);
    }
#endif
}

void AppLauncher::update()
{
    if (process_check_clock_.getElapsedTime().asMilliseconds() > 250) {
#ifdef _WIN32
        if (launched_pid_ > 0) {
            if (Settings::launch.waitForChildProcs) {
                getChildPids(launched_pid_);
            }
            if (!IsProcessRunning(launched_pid_)) {
                spdlog::info("Launched App with PID \"{}\" died", launched_pid_);
                if (Settings::launch.closeOnExit && !Settings::launch.waitForChildProcs) {
                    spdlog::info("Configured to close on exit. Shutting down...");
                    shutdown_();
                }
                launched_pid_ = 0;
            }
        }
        if (Settings::launch.waitForChildProcs) {
            std::erase_if(child_pids_, [](auto pid) {
                const auto running = IsProcessRunning(pid);
                if (!running)
                    spdlog::info("Child process with PID \"{}\" died", pid);
                return !running;
            });
            if (Settings::launch.closeOnExit && child_pids_.empty() && launched_pid_ == 0) {
                spdlog::info("Configured to close on all children exit. Shutting down...");
                shutdown_();
            }
        }
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
                child_pids_.insert(pe.th32ProcessID);
            }
        } while (Process32Next(hp, &pe));
    }
    CloseHandle(hp);
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
        launched_pid_ = process_info.dwProcessId;
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

        // DllInjector::TakeDebugPrivilege();
        HRESULT hResult = S_OK;
        ATL::CComQIPtr<IPackageDebugSettings> debugSettings;
        hResult = debugSettings.CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_ALL);
        debugSettings->EnableDebugging(
            L"Microsoft.MinecraftUWP_1.18.203.0_x64__8wekyb3d8bbwe",
            L"D:\\Alia5\\Documents\\Visual_Studio_Projects\\GlosSI\\x64\\Debug\\GlosSITarget.exe",
            NULL
        );


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

            // Launch the app
            result = sp_app_activation_manager->ActivateApplication(package_full_name, args.empty() ? nullptr : args.data(), AO_NONE, &launched_pid_);
            if (!SUCCEEDED(result)) {
                spdlog::error("ActivateApplication failed: Code {}", result);
            } else {
                spdlog::info(L"Launched UWP Package \"{}\"", package_full_name);
            }
        } else {
            spdlog::error("CoCreateInstance failed: Code {}", result);
        }
        debugSettings->DisableDebugging(L"Microsoft.MinecraftUWP_1.18.203.0_x64__8wekyb3d8bbwe");
        CoUninitialize();
    }
    else {
        spdlog::error("CoInitialize failed: Code {}", result);
    }
}
#endif