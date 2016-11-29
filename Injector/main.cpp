#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include "Injector.h"

#define dllName L"EnforceBindingDLL.dll";

int wmain(int argc, wchar_t* argv[])
{

	if (argc < 2)
	{
		std::wcout << "Missing arguments" << std::endl;
		return 1;
	}

	Injector::TakeDebugPrivilege();

	wchar_t wcPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), wcPath, MAX_PATH);
	std::wstring path(wcPath);
	std::wstring libPath = path.substr(0, path.find_last_of(L"\\")+1) + dllName;

	DWORD pid = NULL;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcsicmp(entry.szExeFile, L"steam.exe") == 0)
			{
				pid = entry.th32ProcessID;
			}
		}
	}
	CloseHandle(snapshot);

	if (pid == NULL)
	{
		std::wcout << "Can't detect Steam.exe running" << std::endl;
		return 1;
	}


	if (std::wstring(argv[1]) == L"--inject")
	{
		int result = Injector::Inject(pid, libPath);
		switch (result)
		{
		case 0:
			std::wcout << "Inject success!" << std::endl;
			break;
		case 1:
			std::wcout << "Error: Couldn't open process" << std::endl;
			break;
		case 2:
			std::wcout << "Error: Couldn't allocate memory" << std::endl;
			break;
		case 3:
			std::wcout << "Error: Couldn't write memory" << std::endl;
			break;
		case 4:
			std::wcout << "Error: Couldn't get pointer ro LoadLibraryW" << std::endl;
			break;
		case 5:
			std::wcout << "Error: Couldn't start remote thread" << std::endl;
			break;
		}
	}
	else if (std::wstring(argv[1]) == L"--eject")
	{
		int result = Injector::Eject(pid, libPath);
		switch (result)
		{
		case 0:
			std::wcout << "Eject success!" << std::endl;
			break;
		case 1:
			std::wcout << "Error: Couldn't open process" << std::endl;
			break;
		case 2:
			std::wcout << "Error: Couldn't find module in process" << std::endl;
			break;
		case 3:
			std::wcout << "Error: Couldn't get pointer ro FreeLibrary" << std::endl;
			break;
		case 4:
			std::wcout << "Error: Couldn't start remote thread" << std::endl;
			break;
		}
	}



	return 0;
}