#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <spdlog/spdlog.h>

#include "util.h"

namespace DllInjector {

inline bool TakeDebugPrivilege()
{
    HANDLE process = GetCurrentProcess(), token;
    TOKEN_PRIVILEGES priv;
    LUID luid = {NULL};

    if (!OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        CloseHandle(process);
        spdlog::error("Failed to open process token");
        return false;
    }
    LookupPrivilegeValue(0, SE_DEBUG_NAME, &luid);

    priv.PrivilegeCount = 1;
    priv.Privileges[0].Luid = luid;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(token, false, &priv, sizeof(priv), NULL, NULL)) {
        spdlog::error("Failed to take debug privilege token");
    }

    CloseHandle(token);
    CloseHandle(process);
    return true;
}

inline bool Inject(DWORD pid, const std::wstring& lib_path)
{
    HANDLE process = NULL, alloc_address = NULL, remote_thread = NULL;
    LPTHREAD_START_ROUTINE thread_routine = NULL;

    size_t path_size = (lib_path.length() + 1) * sizeof(wchar_t);

    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, false, pid);

    if (!process) {
        spdlog::error("Failed to open process");
        return false;
    }

    alloc_address = VirtualAllocEx(process, NULL, path_size, MEM_COMMIT, PAGE_READWRITE);

    if (!VirtualAllocEx(process, NULL, path_size, MEM_COMMIT, PAGE_READWRITE)) {
        spdlog::error("Failed to allocate memory in target process");
        CloseHandle(process);
        return false;
    }

    if (!WriteProcessMemory(process, (LPVOID)alloc_address, lib_path.c_str(), path_size, NULL)) {
        spdlog::error("Failed to write memory in target process");
        VirtualFreeEx(process, alloc_address, path_size, MEM_DECOMMIT);
        CloseHandle(process);
        return false;
    }

    thread_routine = reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryW"));
    if (!thread_routine) {
        spdlog::error("Failed to get address of LoadLibraryW");
        VirtualFreeEx(process, alloc_address, path_size, MEM_DECOMMIT);
        CloseHandle(process);
        return false;
    }

    remote_thread = CreateRemoteThread(process, NULL, 0, thread_routine, alloc_address, 0, NULL);

    if (!remote_thread) {
        spdlog::error("Failed to create remote thread");
        VirtualFreeEx(process, alloc_address, path_size, MEM_DECOMMIT);
        CloseHandle(process);
        return false;
    }

    WaitForSingleObject(remote_thread, INFINITE);

    CloseHandle(remote_thread);
    VirtualFreeEx(process, alloc_address, path_size, MEM_DECOMMIT);
    CloseHandle(process);
    spdlog::debug("Successfully injected");
    return true;
}

inline bool findModule(DWORD pid, std::wstring& lib_path, HMODULE& hMod)
{
    MODULEENTRY32W entry = {sizeof(entry)};
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

    if (Module32FirstW(snapshot, &entry) == TRUE) {
        while (Module32NextW(snapshot, &entry) == TRUE) {
            std::wstring ModuleName(entry.szModule);
            std::wstring ExePath(entry.szExePath);
            if (ModuleName == lib_path || ExePath == lib_path) {
                hMod = (HMODULE)entry.modBaseAddr;
                CloseHandle(snapshot);
                return true;
            }
        }
    }
    CloseHandle(snapshot);
    spdlog::error(L"Failed to find module \"{}\"", lib_path);
    return false;
}

inline void injectDllInto(std::filesystem::path dllPath, const std::wstring& processName)
{
    if (std::filesystem::exists(dllPath)) {
        const auto explorer_pid = glossi_util::PidByName(processName);
        if (explorer_pid != 0) {
            if (DllInjector::TakeDebugPrivilege()) {
                // No need to eject, as the dll is self-ejecting.
                if (DllInjector::Inject(explorer_pid, dllPath.wstring())) {
                    spdlog::info(L"Successfully injected {} into {}", dllPath.filename().wstring(), processName);
                }
            }
        }
        else {
            spdlog::error(L"{} not found", processName); // needs loglevel WTF
        }
    }
    else {
        spdlog::error(L"{} not found", dllPath.wstring());
    }
}

}; // namespace DllInjector
