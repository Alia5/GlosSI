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

ULONG VirtualControllerThread::ulTargetSerials[XUSER_MAX_COUNT];

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
		VirtualControllerThread::ulTargetSerials[i] = NULL;
	}

}


VirtualControllerThread::~VirtualControllerThread()
{
	stop();
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
	controllerThread.join();
	for (int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		vigem_target_unplug(&vtX360[i]);
	}
}

void VirtualControllerThread::resetControllers()
{
	iRealControllers = getRealControllers();
}

bool VirtualControllerThread::isRunning()
{
	return bShouldRun;
}

void VirtualControllerThread::controllerLoop()
{
	DWORD result;
	sf::Clock reCheckControllerTimer;
	int i, j;
	while (bShouldRun)
	{
		sfClock.restart();

		if (reCheckControllerTimer.getElapsedTime().asSeconds() >= 1.f)
		{
			iTotalControllers = 0;
			for (i = 0; i < XUSER_MAX_COUNT; i++)
			{
				ZeroMemory(&xsState[i], sizeof(XINPUT_STATE));

				result = XInputGetState(i, &xsState[i]);

				if (result == ERROR_SUCCESS)
				{
					iTotalControllers++;
				}
				else {
					break;
				}
			}
			iTotalControllers -= iVirtualControllers;
			reCheckControllerTimer.restart();
		}

		for (i = iRealControllers; i < iTotalControllers && i < XUSER_MAX_COUNT; i++)
		{
			////////
			ZeroMemory(&xsState[i], sizeof(XINPUT_STATE));


			result = XInputGetState(i, &xsState[i]);

			if (result == ERROR_SUCCESS)
			{

				if (VIGEM_SUCCESS(vigem_target_plugin(Xbox360Wired, &vtX360[i])))
				{
					iVirtualControllers++;

					std::cout << "Plugged in controller " << vtX360[i].SerialNo << std::endl;

					VirtualControllerThread::ulTargetSerials[i] = vtX360[i].SerialNo;

					vigem_register_xusb_notification((PVIGEM_XUSB_NOTIFICATION)&VirtualControllerThread::controllerCallback, vtX360[i]);
				}

				RtlCopyMemory(&xrReport[i], &xsState[i].Gamepad, sizeof(XUSB_REPORT));

				vigem_xusb_submit_report(vtX360[i], xrReport[i]);
			}
			else
			{
				if (VIGEM_SUCCESS(vigem_target_unplug(&vtX360[i])))
				{
					iVirtualControllers--;
					iTotalControllers = 0;
					for (j = 0; j < XUSER_MAX_COUNT; j++)
					{
						ZeroMemory(&xsState[j], sizeof(XINPUT_STATE));

						result = XInputGetState(j, &xsState[j]);

						if (result == ERROR_SUCCESS)
						{
							iTotalControllers++;
						}
						else {
							break;
						}
					}
					iTotalControllers -= iVirtualControllers;
					std::cout << "Unplugged controller " << vtX360[i].SerialNo << std::endl;
					VirtualControllerThread::ulTargetSerials[i] = NULL;
				}
			}
		}

		tickTime = sfClock.getElapsedTime().asMicroseconds();
		//std::cout << tickTime << std::endl;
		if (tickTime < delay)
			std::this_thread::sleep_for(std::chrono::microseconds(delay-tickTime));
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
	std::cout << "Target Serial: " << Target.SerialNo
		<< "; LMotor: " << (unsigned int)(LargeMotor * 0xff) << "; "
		<< " SMotor: " << (unsigned int)(SmallMotor * 0xff) << "; " << std::endl;

	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = LargeMotor * 0xff; //Controllers only use 1 byte, XInput-API uses two, ViGEm also only uses one, like the hardware does, so we have to multiply
	vibration.wRightMotorSpeed = SmallMotor * 0xff;


	for (int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		if (VirtualControllerThread::ulTargetSerials[i] == Target.SerialNo)
		{
			XInputSetState(i, &vibration);
		}
	}
}
