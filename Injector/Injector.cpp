#include "Injector.h"

#include <iostream>

void Injector::TakeDebugPrivilege()
{
	HANDLE hProcess = GetCurrentProcess(), hToken;
	TOKEN_PRIVILEGES priv;
	LUID luid = { NULL };

	OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(0, SE_DEBUG_NAME, &luid);

	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;			
	AdjustTokenPrivileges(hToken, false, &priv, sizeof(priv), NULL, NULL);
																																
	CloseHandle(hToken);											
	CloseHandle(hProcess);
}

int Injector::Inject(DWORD pid, std::wstring &libPath)
{
	HANDLE hProcess = NULL, allocAddress = NULL, hRemoteThread = NULL;
	LPTHREAD_START_ROUTINE pfnThreadRtn = NULL;

	size_t pathSize = (libPath.length() + 1) * sizeof(wchar_t);

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, false, pid);

	if (!hProcess)
		return 1;

	allocAddress = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);

	if (!allocAddress)
	{
		CloseHandle(hProcess);
		return 2;
	}

	if (!WriteProcessMemory(hProcess, (LPVOID)allocAddress, libPath.c_str(), pathSize, NULL))
	{
		VirtualFreeEx(hProcess, allocAddress, pathSize, MEM_DECOMMIT);
		CloseHandle(hProcess);
		return 3;
	}

	pfnThreadRtn = reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryW"));
	if (!pfnThreadRtn)
	{
		VirtualFreeEx(hProcess, allocAddress, pathSize, MEM_DECOMMIT);
		CloseHandle(hProcess);
		return 4;
	}

	hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, allocAddress, 0, NULL);

	if (!hRemoteThread)
	{
		VirtualFreeEx(hProcess, allocAddress, pathSize, MEM_DECOMMIT);
		CloseHandle(hProcess);
		return 5;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);

	CloseHandle(hRemoteThread);
	VirtualFreeEx(hProcess, allocAddress, pathSize, MEM_DECOMMIT);
	CloseHandle(hProcess);

	return 0;
}

int Injector::Eject(DWORD pid, std::wstring &libPath)
{
	HANDLE hProcess = NULL, hRemoteThread = NULL;
	HMODULE hLibMod = NULL;
	LPTHREAD_START_ROUTINE pfnThreadRtn = NULL;


	if (!findModule(pid, libPath, hLibMod))
		return 2;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION, false, pid);
	if (!hProcess)
		return 1;

	pfnThreadRtn = (PTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandle(L"Kernel32"), "FreeLibrary");

	if (!pfnThreadRtn)
	{
		CloseHandle(hProcess);
		return 3;
	}

	hRemoteThread = CreateRemoteThread(hProcess, NULL, NULL, pfnThreadRtn, hLibMod, NULL, NULL);

	if (!hRemoteThread)
		return 4;

	WaitForSingleObject(hRemoteThread, INFINITE);

	CloseHandle(hProcess);

	return 0;
}

bool Injector::findModule(DWORD pid, std::wstring &libPath, HMODULE &hMod)
{
	MODULEENTRY32W entry = { sizeof(entry) };
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

	if (Module32FirstW(snapshot, &entry) == TRUE)
	{
		while (Module32NextW(snapshot, &entry) == TRUE)
		{
			std::wstring ModuleName(entry.szModule);
			std::wstring ExePath(entry.szExePath);
			if (ModuleName == libPath || ExePath == libPath)
			{
				hMod = (HMODULE)entry.modBaseAddr;
				CloseHandle(snapshot);
				return true;
			}
		}
	}
	CloseHandle(snapshot);
	return false;
}

