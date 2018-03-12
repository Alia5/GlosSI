#pragma once
#include <Windows.h>
#include <tlhelp32.h>

namespace process_alive
{
	//stolen from: https://stackoverflow.com/questions/1591342/c-how-to-determine-if-a-windows-process-is-running
	inline bool IsProcessRunning(const wchar_t *processName)
	{
		bool exists = false;
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry))
			do {
				if (!_wcsicmp(entry.szExeFile, processName))
				{
					exists = true;
					break;
				}
			} while (Process32Next(snapshot, &entry));

			CloseHandle(snapshot);
			return exists;
	}

	inline BOOL IsProcessRunning(DWORD pid)
	{
		const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
		if (process == nullptr)
			return false;
		const DWORD ret = WaitForSingleObject(process, 0);
		CloseHandle(process);
		return ret == WAIT_TIMEOUT;
	}
}