#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <string>

class Injector {


public:
	static void TakeDebugPrivilege();
	static int Inject(DWORD pid, std::wstring &libPath);
	static int Eject(DWORD pid, std::wstring &libPath);

private:

	static bool findModule(DWORD pid, std::wstring &libPath, HMODULE &hMod);


};