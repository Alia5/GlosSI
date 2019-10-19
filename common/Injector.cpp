/*
Copyright 2018-2019 Peter Repukat - FlatspotSoftware

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
#include "Injector.h"
#include <tlhelp32.h>
#include <iostream>

#include "../common/loguru.hpp"

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
	{
		return 1;

	}

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
	{
		return 2;
	}
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION, false, pid);
	if (!hProcess)
	{
		return 1;
	}
	pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32"), "FreeLibrary");

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



int Injector::hookSteam()
{
	Injector::TakeDebugPrivilege();

	wchar_t wcPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), wcPath, MAX_PATH);
	std::wstring path(wcPath);
	std::wstring libPath = path.substr(0, 1 + path.find_last_of(L'\\')) + L"EnforceBindingDLL.dll";

	DWORD pid = NULL;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_wcsicmp(entry.szExeFile, L"steam.exe") == 0)
			{
				pid = entry.th32ProcessID;
			}
		}
	}
	CloseHandle(snapshot);

	if (pid == NULL)
	{
		LOG_F(WARNING, "Can't detect Steam.exe running");
		return 0;
	}
	const int result = Injector::Inject(pid, libPath);
	switch (result)
	{
	case 0:
		LOG_F(INFO, "Inject success!");
		return 1;
	case 1:
		LOG_F(ERROR, "Couldn't open process");
		break;
	case 2:
		LOG_F(ERROR, "Couldn't allocate memory");
		break;
	case 3:
		LOG_F(ERROR, "Couldn't write memory");
		break;
	case 4:
		LOG_F(ERROR, "Couldn't get pointer ro LoadLibraryW");
		break;
	case 5:
		LOG_F(ERROR, "Couldn't start remote thread");
		break;
	default:
		return 0;
	}
}

int Injector::unhookSteam()
{
	Injector::TakeDebugPrivilege();

	wchar_t wcPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), wcPath, MAX_PATH);
	std::wstring path(wcPath);
	std::wstring libPath = path.substr(0, 1 + path.find_last_of(L'\\')) + L"EnforceBindingDLL.dll";

	DWORD pid = NULL;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_wcsicmp(entry.szExeFile, L"steam.exe") == 0)
			{
				pid = entry.th32ProcessID;
			}
		}
	}
	CloseHandle(snapshot);

	if (pid == NULL)
	{
		LOG_F(WARNING, "Can't detect Steam.exe running");
		return 0;
	}

	const int result = Injector::Eject(pid, libPath);
	switch (result)
	{
	case 0:
		LOG_F(INFO, "Eject success!");
		return 1;
	case 1:
		LOG_F(ERROR, "Couldn't open process");
		break;
	case 2:
		LOG_F(ERROR, "Couldn't find module in process");
		break;
	case 3:
		LOG_F(ERROR, "Couldn't get pointer ro FreeLibrary");
		break;
	case 4:
		LOG_F(ERROR, "Couldn't start remote thread");
		break;
	default:
		return 0;
	}

}