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
	resetControllers();

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

void VirtualControllerThread::resetControllers()
{
	//iRealControllers = getRealControllers();
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
	//int i, j;
	while (bShouldRun)
	{
		sfClock.restart();

		if (realXGetState == nullptr && testTimer.getElapsedTime().asSeconds() > 1)
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

			uint64_t testaddr = reinterpret_cast<uint64_t>(GetProcAddress(xinputmod, "XInputGetState"));

			std::cout << "testaddr: " << std::hex << testaddr << "\n";

			XInputGetState_t realXgstate = reinterpret_cast<XInputGetState_t>(testaddr);

			std::cout << "realXgstate: " << std::hex << realXgstate << "\n";
			for (int i = 0; i < 5; i++)
			{
				valveHookBytes[i] = *reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(*realXgstate) + i);
			}

			realXGetState = realXgstate;
		}

		if (realXGetState != nullptr)
		{
			if (!checkedControllers)
			{
				for (int i = 0; i < XUSER_MAX_COUNT; i++)
				{
					XINPUT_STATE state = { 0 };
					result = XInputGetStateWrapper(i, &state);
					result2 = callRealXinputGetState(i, &state);
					if (result != result2)
						controllerCount++;
				}
				std::cout << "ControllerCount: " << std::to_string(controllerCount) << "\n";
				checkedControllers = true;
			}

			for (int i = 0; i < controllerCount; i++)
			{
				////////
				ZeroMemory(&xsState[i], sizeof(XINPUT_STATE));
				result = XInputGetStateWrapper(i, &xsState[i]);

				if (result == ERROR_SUCCESS)
				{
					vigem_target_set_vid(&vtX360[i], 0x1234);
					vigem_target_set_pid(&vtX360[i], 0x0001);

					if (VIGEM_SUCCESS(vigem_target_plugin(Xbox360Wired, &vtX360[i])))
					{
						std::cout << "Plugged in controller " << vtX360[i].SerialNo << std::endl;
						vigem_register_xusb_notification(reinterpret_cast<PVIGEM_XUSB_NOTIFICATION>(&VirtualControllerThread::controllerCallback), vtX360[i]);
					}

					vigem_xusb_submit_report(vtX360[i], *reinterpret_cast<XUSB_REPORT*>(&xsState[i].Gamepad));
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

		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int VirtualControllerThread::getRealControllers()
{
	int realControllers = 0;
	UINT numDevices = NULL;

	GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));

	PRAWINPUTDEVICELIST rawInputDeviceList;
	rawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(sizeof(RAWINPUTDEVICELIST) * numDevices);
	GetRawInputDeviceList(rawInputDeviceList, &numDevices, sizeof(RAWINPUTDEVICELIST));

	for (unsigned int i = 0; i < numDevices; i++)
	{
		RID_DEVICE_INFO devInfo;
		devInfo.cbSize = sizeof(RID_DEVICE_INFO);
		GetRawInputDeviceInfo(rawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &devInfo, (PUINT)&devInfo.cbSize);
		if (devInfo.hid.dwVendorId == 0x45e && devInfo.hid.dwProductId == 0x28e)
		{
			realControllers++;
		}

	}

	free(rawInputDeviceList);
	std::cout << "Detected " << realControllers << " real connected X360 Controllers" << std::endl;
	return realControllers;
}

void VirtualControllerThread::controllerCallback(VIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = LargeMotor * 0xff; //Controllers only use 1 byte, XInput-API uses two, ViGEm also only uses one, like the hardware does, so we have to multiply
	vibration.wRightMotorSpeed = SmallMotor * 0xff; //Yeah yeah I do know about bitshifting and the multiplication not being 100% correct...


	//for (int i = 0; i < XUSER_MAX_COUNT; i++)
	//{
	//	if (VirtualControllerThread::ulTargetSerials[i] == Target.SerialNo)
	//	{
			XInputSetState(Target.SerialNo-1, &vibration);
	//	}
	//}
}

DWORD VirtualControllerThread::XInputGetStateWrapper(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	return XInputGetState(dwUserIndex, pState);
}

DWORD VirtualControllerThread::callRealXinputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	DWORD ret;
	DWORD dwOldProtect, dwBkup;

	BYTE* Address = reinterpret_cast<BYTE*>(realXGetState);
	VirtualProtect(Address, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	for (DWORD i = 0; i < 5; i++)
	{
		*(Address + i) = realBytes[i];
	}
	VirtualProtect(Address, 4, dwOldProtect, &dwBkup);

	ret = realXGetState(dwUserIndex, pState);

	VirtualProtect(Address, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	for (int i = 0; i < 5; i++)
	{
		*(Address + i) = valveHookBytes[i];
	}
	VirtualProtect(Address, 5, dwOldProtect, &dwBkup);

	return ret;
}


