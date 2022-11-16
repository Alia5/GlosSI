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
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tlhelp32.h>

namespace glossi_util {

inline DWORD PidByName(const std::wstring& name)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (std::wstring(entry.szExeFile).find(name) != std::string::npos) {
                return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return 0;
}

inline std::wstring GetProcName(DWORD pid)
{
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);
    const HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE) {
        spdlog::trace("util::GetProcName: can't get a process snapshot");
        return L"";
    }

    for (BOOL bok = Process32First(processesSnapshot, &processInfo);
         bok;
         bok = Process32Next(processesSnapshot, &processInfo)) {
        if (pid == processInfo.th32ProcessID) {
            CloseHandle(processesSnapshot);
            return processInfo.szExeFile;
        }
    }
    CloseHandle(processesSnapshot);
    return L"";
}

inline bool KillProcess(DWORD pid)
{
    auto res = true;
    if (const auto proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid)) {
        spdlog::debug("Terminating process: {}", pid);
        res = TerminateProcess(proc, 0);
        if (!res) {
            spdlog::error("Failed to terminate process: {}", pid);
        }
        CloseHandle(proc);
    }
    return res;
}

} // namespace glossi_util
