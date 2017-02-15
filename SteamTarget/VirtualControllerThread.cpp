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
#include "VirtualControllerThread.h"
//

VirtualControllerThread::VirtualControllerThread()
{
	if (!VIGEM_SUCCESS(vigem_init()))
	{
		std::cout << "Error initializing ViGem!" << std::endl;
		MessageBoxW(NULL, L"Error initializing ViGem!", L"GloSC-SteamTarget", MB_OK);
		bShouldRun = false;
	}

	VIGEM_TARGET vtX360[XUSER_MAX_COUNT];
	for (int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		VIGEM_TARGET_INIT(&vtX360[i]);
	}
}


VirtualControllerThread::~VirtualControllerThread()
{
	controllerThread.join();
	vigem_shutdown();
}

void VirtualControllerThread::run()
{
	bShouldRun = true;
	controllerThread = std::thread(&VirtualControllerThread::controllerLoop, this);
}

void VirtualControllerThread::stop()
{
	bShouldRun = false;
	for (int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		vigem_target_unplug(&vtX360[i]);
	}
}


bool VirtualControllerThread::isRunning()
{
	return bShouldRun;
}

void VirtualControllerThread::controllerLoop()
{
	DWORD result;
	DWORD result2;
	sf::Clock testTimer;
	while (bShouldRun)
	{
		sfClock.restart();

		// We have to retrieve the XInputGetState function by loading it via GetProcAdress
		// otherwise the M$ compiler calls to a jumptable, jumping to the real function
		// We can't have this if we wan't to dynamically unpatch and repatch Valve's XInput hook
		// Also wait a second, jut to be sure Steam has done it's hooking thing...
		if (XGetState == nullptr && testTimer.getElapsedTime().asSeconds() > 1)
		{
			HMODULE xinputmod = nullptr;

			HANDLE hProcess = GetCurrentProcess();
			HMODULE hMods[1024];
			DWORD cbNeeded;
			EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded);
			for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				TCHAR szModName[MAX_PATH];

				if (GetModuleBaseName(hProcess, hMods[i], szModName,
					sizeof(szModName) / sizeof(TCHAR)))
				{
					std::wstring name(&szModName[0]);
					auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
					f.tolower(&name[0], &name[0] + name.size());
					if (name.find(std::wstring(L"xinput")) != std::wstring::npos)
					{
						xinputmod = hMods[i];
						break;
					}
				}
			}

			XInputGetState_t realXgstate = reinterpret_cast<XInputGetState_t>(GetProcAddress(xinputmod, "XInputGetState"));

			std::cout << "realXgstate: " << std::hex << realXgstate << "\n";
			for (int i = 0; i < 5; i++)
			{
				valveHookBytes[i] = *reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(*realXgstate) + i);
			}

			XGetState = realXgstate;
			controllerCount = 1;
		}

		if (XGetState != nullptr)
		{
			for (int i = 0; i < XUSER_MAX_COUNT; i++)
			{
				////////
				XINPUT_STATE state = { 0 };
				result = XInputGetStateWrapper(i, &state);
				XINPUT_STATE state2 = { 0 };
				result2 = callRealXinputGetState(i, &state2);

				if (result == ERROR_SUCCESS)
				{
					if (result2 != ERROR_SUCCESS)
					{
						// By using VID and PID of Valve's SteamController, Steam doesn't give us ANOTHER "fake" XInput device
						// Leading to endless pain and suffering. 
						// Or really, leading to pluggin in one virtual controller after another and mirroring inputs
						// Also annoying the shit out of the user when they open the overlay as steam prompts to setup new XInput devices
						// Also avoiding any fake inputs from Valve's default controllerprofile
						// -> Leading to endless pain and suffering
						vigem_target_set_vid(&vtX360[i], 0x28de); //Valve SteamController VID
						vigem_target_set_pid(&vtX360[i], 0x1102); //Valve SteamController PID

						int vigem_res = vigem_target_plugin(Xbox360Wired, &vtX360[i]);
						if (vigem_res == VIGEM_ERROR_TARGET_UNINITIALIZED)
						{
							VIGEM_TARGET_INIT(&vtX360[i]);
						}
						if (vigem_res == VIGEM_ERROR_NONE)
						{
							std::cout << "Plugged in controller " << vtX360[i].SerialNo << std::endl;
							vigem_register_xusb_notification(reinterpret_cast<PVIGEM_XUSB_NOTIFICATION>(&VirtualControllerThread::controllerCallback), vtX360[i]);
						}
					}

					vigem_xusb_submit_report(vtX360[i], *reinterpret_cast<XUSB_REPORT*>(&state.Gamepad));
				}
				else
				{
					if (VIGEM_SUCCESS(vigem_target_unplug(&vtX360[i])))
					{
						std::cout << "Unplugged controller " << vtX360[i].SerialNo << std::endl;
					}
				}
			}
		}

		tickTime = sfClock.getElapsedTime().asMicroseconds();
		if (tickTime < delay)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(delay - tickTime));
		}

	}
}

void VirtualControllerThread::controllerCallback(VIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = LargeMotor * 0xff; //Controllers only use 1 byte, XInput-API uses two, ViGEm also only uses one, like the hardware does, so we have to multiply
	vibration.wRightMotorSpeed = SmallMotor * 0xff; //Yeah yeah I do know about bitshifting and the multiplication not being 100% correct...

	XInputSetState(Target.SerialNo-1, &vibration);
}

DWORD VirtualControllerThread::XInputGetStateWrapper(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	return XInputGetState(dwUserIndex, pState);
}

DWORD VirtualControllerThread::callRealXinputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	DWORD ret;
	DWORD dwOldProtect, dwBkup;

	BYTE* Address = reinterpret_cast<BYTE*>(XGetState);
	VirtualProtect(Address, opPatchLenght, PAGE_EXECUTE_READWRITE, &dwOldProtect);		//Change permissions of memory..
	for (DWORD i = 0; i < opPatchLenght; i++)											//unpatch Valve's hook
	{
		*(Address + i) = realBytes[i];
	}
	//VirtualProtect(Address, opPatchLenght, dwOldProtect, &dwBkup);						//Revert permission change...

	ret = XGetState(dwUserIndex, pState);												//Cal REAL XInputGetState...

	//VirtualProtect(Address, opPatchLenght, PAGE_EXECUTE_READWRITE, &dwOldProtect);      //Change permissions of memory..
	for (int i = 0; i < opPatchLenght; i++)												//repatch Valve's hook
	{
		*(Address + i) = valveHookBytes[i];
	}
	VirtualProtect(Address, opPatchLenght, dwOldProtect, &dwBkup);						//Revert permission change...

	return ret;
}


