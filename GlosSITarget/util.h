#pragma once

#include "DllInjector.h"

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


} // namespace glossi_util
