/*
Copyright 2018 Peter Repukat - FlatspotSoftware

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


#include "../dependencies/minhook/include/MinHook.h"

#if defined _M_X64
#pragma comment(lib, "../packages/minhook.1.3.3/lib/native/lib/libMinHook-x64-v141-mt.lib")
#elif defined _M_IX86
#pragma comment(lib, "../dependencies/minhook/build/VC15/lib/Release/libMinHook.x86.lib")
#endif

namespace fgwinhook
{

	typedef HWND(WINAPI *GETFOREGROUNDWINDOW)();

	GETFOREGROUNDWINDOW fGetForegroundWindow = NULL;

	HWND WINAPI DetourGetForegroundWindow()
	{
		return FindWindow(nullptr, L"GloSC_OverlayWindow");
	}


	template <typename T>
	inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
	{
		return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
	}

	template <typename T>
	inline MH_STATUS MH_CreateHookApiEx(
		LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, T** ppOriginal)
	{
		return MH_CreateHookApi(
			pszModule, pszProcName, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
	}


	void patchForegroundWindow()
	{
		// Initialize MinHook.
		if (MH_Initialize() != MH_OK)
			return;

		// Create a hook for GetActiveWindow, in disabled state.
		if (MH_CreateHookApiEx(L"user32", "GetForegroundWindow", &DetourGetForegroundWindow, &fGetForegroundWindow) != MH_OK)
			return;
		// Enable the hook for GetActiveWindow.

		if (MH_EnableHook(&GetForegroundWindow) != MH_OK)
			return;
	}

	void unpatchForegroundWindow()
	{
		if (MH_DisableHook(&GetForegroundWindow) != MH_OK)
			return;
		// Uninitialize MinHook.
		if (MH_Uninitialize() != MH_OK)
			return;
	}

}