/*
Copyright 2016 Peter Repukat - FlatspotSoftware

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


#include <stdint.h>

#include <Windows.h>
#include <psapi.h>

#include "../packages/minhook.1.3.3/lib/native/include/MinHook.h"

#if defined _M_X64
#pragma comment(lib, "../packages/minhook.1.3.3/lib/native/lib/libMinHook-x64-v141-mt.lib")
#elif defined _M_IX86
#pragma comment(lib, "../packages/minhook.1.3.3/lib/native/lib/libMinHook-x86-v141-mt.lib")
#endif


class EnforceBindings
{
public:

	static void patchBytes();
	static void Unpatch();

	static void patchLizard();
	static void unpatchLizard();

private:

	static void PlaceJMP(BYTE * Address, DWORD jumpTo, DWORD lenght);
	static void RestoreBytes(BYTE *Address, BYTE *original, DWORD lenght);
	static MODULEINFO GetModInfo(char *szModule);
	static DWORD FindPattern(char *module, const char *pattern, const char *mask);
	static DWORD FindHookFunctionAdress();
};

