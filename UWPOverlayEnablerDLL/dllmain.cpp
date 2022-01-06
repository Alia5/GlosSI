/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

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

/*

There are two (known to me, at time of writing) ways to get a working overlay for UWP Apps
1. Create an accessibility app
	Set UIAcces in manifest to true
	This however requires that the application is digitally signed
	and is run from a trusted directory (Program Files; System32)
	At this point UWP overlays are not a technical issue anymore, but a monetary
	I have no interest in spending ~100 bucks a year just to provide this functionality to users.

	You could also self-sign the application, but installing a trusted root CA is a security risk

2.  Use undocumented SetWindowBand function
	This function however is not freely callable from every process.
	Even when injected into explorer.exe, it doesn't seem to work when just calling normally...
	So let's hook the original function, and try to do "the magic" then
	This seemingly works ¯\_(ツ)_/¯
	
	"The magic":
		Hook SetWindowBand
		Once called, find GlosSI Window
		Set GlosSI Window to ZBID_SYSTEM_TOOLS (Doesn't seem to require any special stuff)
		Self-Eject
		
		**PROFIT!**

*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define SUBHOOK_STATIC
#include <atomic>
#include <subhook.h>

enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};
typedef BOOL(WINAPI* fSetWindowBand)(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand);


subhook::Hook SetWindowBandHook;
fSetWindowBand SetWindowBand;

std::atomic<bool> allow_exit = false;

BOOL WINAPI SetGlosSIWindowBand(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand)
{
	subhook::ScopedHookRemove remove(&SetWindowBandHook);
	const auto glossi_hwnd = FindWindowA(nullptr, "GlosSITarget");
	if (glossi_hwnd)
	{
		// Most window bands don't really seem to work.
		// However, notification and system_tools does!
		// use system tools, as that allows the steam overlay to be interacted with
		// without UWP apps minimizing
		SetWindowBand(glossi_hwnd, nullptr, ZBID_SYSTEM_TOOLS);
		allow_exit = true;
	}
	return SetWindowBand(hWnd, hwndInsertAfter, dwBand);
}

DWORD WINAPI WaitThread(HMODULE hModule)
{
	while (!allow_exit)
	{
		Sleep(10);
	}
	if (SetWindowBandHook.IsInstalled())
		SetWindowBandHook.Remove();
	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		const auto hpath = LoadLibrary(L"user32.dll");
		if (hpath)
		{
			SetWindowBand = reinterpret_cast<fSetWindowBand>(GetProcAddress(hpath, "SetWindowBand"));
			SetWindowBandHook.Install(GetProcAddress(hpath, "SetWindowBand"), &SetGlosSIWindowBand, subhook::HookFlags::HookFlag64BitOffset);
			CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)WaitThread, hModule, 0, nullptr));
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		if (SetWindowBandHook.IsInstalled())
			SetWindowBandHook.Remove();
	}
    return TRUE;
}

