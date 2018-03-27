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
#include "VirtualControllerThread.h"


#include "../common/loguru.hpp"


VirtualControllerThread::VirtualControllerThread(const int delay)
{
	driver_ = vigem_alloc();

	if (!VIGEM_SUCCESS(vigem_connect(driver_)))
	{
		LOG_F(ERROR, "initializing ViGem!");
		MessageBoxW(NULL, L"Error initializing ViGem!", L"GloSC-SteamTarget", MB_OK);
		b_should_run_ = false;
	}

	for (auto & target : vt_x360_)
	{
		target = vigem_target_x360_alloc();
	}

	seven_ = IsWindows7OrGreater() != IsWindows8OrGreater();
	delay_ = delay;
}


VirtualControllerThread::~VirtualControllerThread()
{
	if (controller_thread_.joinable())
		controller_thread_.join();
	vigem_disconnect(driver_);
}

void VirtualControllerThread::run()
{
	b_should_run_ = true;
	controller_thread_ = std::thread(&VirtualControllerThread::controllerLoop, this);
}

void VirtualControllerThread::stop()
{
	b_should_run_ = false;
	for (auto & target : vt_x360_)
	{
		vigem_target_remove(driver_, target);
	}
}


bool VirtualControllerThread::isRunning() const
{
	return b_should_run_;
}

void VirtualControllerThread::controllerLoop()
{
	sf::Clock waitForHookTimer;
	while (b_should_run_)
	{
		sf_clock_.restart();

		// We have to retrieve the XInputGetState function by loading it via GetProcAdress
		// otherwise we get calls to a jumptable, jumping to the real function
		// We can't have this if we wan't to dynamically unpatch and repatch Valve's XInput hook
		// Also wait a second, jut to be sure Steam has done it's hooking thing...
		if (x_get_state_ == nullptr && waitForHookTimer.getElapsedTime().asSeconds() > 1)
		{
			HMODULE xinputmod = nullptr;

			const HANDLE hProcess = GetCurrentProcess();
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

			const XInputGetState_t realXgstate = reinterpret_cast<XInputGetState_t>(GetProcAddress(xinputmod, "XInputGetState"));

			//std::cout << "realXgstate: " << std::hex << realXgstate << "\n";
			for (int i = 0; i < 5; i++)
			{
				valve_hook_bytes_[i] = *reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(*realXgstate) + i);
			}

			x_get_state_ = realXgstate;
			controller_count_ = 1;
		}

		if (x_get_state_ != nullptr)
		{
			for (int i = 0; i < XUSER_MAX_COUNT; i++)
			{
				////////

				//Call the hooked, as well as the 'real' XInputGetState function to determine if a controller is real of 'fake' (from Steam)
				XINPUT_STATE state = { 0 };
				const DWORD result = XInputGetStateWrapper(i, &state);
				XINPUT_STATE state2 = { 0 };
				const DWORD result2 = callRealXinputGetState(i, &state2);

				if (result == ERROR_SUCCESS)
				{
					if ( (result2 != ERROR_SUCCESS) == seven_ ) //for whatever reason, the second call also returns true on win7, false (as it should(?)) otherwise.
					{
						// By using VID and PID of Valve's SteamController, Steam doesn't give us ANOTHER "fake" XInput device
						// Leading to endless pain and suffering. 
						// Or really, leading to pluggin in one virtual controller after another and mirroring inputs
						// Also annoying the shit out of the user when they open the overlay as steam prompts to setup new XInput devices
						// Also avoiding any fake inputs from Valve's default controllerprofile
						// -> Leading to endless pain and suffering
						vigem_target_set_vid(vt_x360_[i], 0x28de); //Valve SteamController VID
						vigem_target_set_pid(vt_x360_[i], 0x1102); //Valve SteamController PID

						const int vigem_res = vigem_target_add(driver_, vt_x360_[i]);
						if (vigem_res == VIGEM_ERROR_TARGET_UNINITIALIZED)
						{
							vt_x360_[i] = vigem_target_x360_alloc();
						}
						if (vigem_res == VIGEM_ERROR_NONE)
						{
							LOG_F(INFO, "Plugged in controller %d", vigem_target_get_index(vt_x360_[i]));
							vigem_target_x360_register_notification(driver_, vt_x360_[i],
							                                        reinterpret_cast<PVIGEM_X360_NOTIFICATION>(&VirtualControllerThread::
								                                        controllerCallback));
						}
					}

					if (vt_x360_[i] != nullptr)
						vigem_target_x360_update(driver_, vt_x360_[i], *reinterpret_cast<XUSB_REPORT*>(&state.Gamepad));
				}
				else
				{
					if (VIGEM_SUCCESS(vigem_target_remove(driver_, vt_x360_[i])))
					{
						LOG_F(INFO, "Unplugged controller %d", vigem_target_get_index(vt_x360_[i]));
					}
				}
			}
		}

		tick_time_ = sf_clock_.getElapsedTime().asMicroseconds();
		if (tick_time_ < delay_)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(delay_ - tick_time_));
		}

	}
}

void VirtualControllerThread::controllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = LargeMotor * 0xff; //Controllers only use 1 byte, XInput-API uses two, ViGEm also only uses one, like the hardware does, so we have to multiply
	vibration.wRightMotorSpeed = SmallMotor * 0xff; //Yeah yeah I do know about bitshifting and the multiplication not being 100% correct...

	XInputSetState(vigem_target_get_index(Target)-1, &vibration);
}

DWORD VirtualControllerThread::XInputGetStateWrapper(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	return XInputGetState(dwUserIndex, pState);
}

DWORD VirtualControllerThread::callRealXinputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	DWORD dwOldProtect, dwBkup;

	auto* Address = reinterpret_cast<BYTE*>(x_get_state_);
	VirtualProtect(Address, op_patch_lenght, PAGE_EXECUTE_READWRITE, &dwOldProtect);		//Change permissions of memory..
	for (DWORD i = 0; i < op_patch_lenght; i++)											//unpatch Valve's hook
	{
		*(Address + i) = real_bytes[i];
	}

	const DWORD ret = x_get_state_(dwUserIndex, pState);												//Cal REAL XInputGetState...

	for (int i = 0; i < op_patch_lenght; i++)												//repatch Valve's hook
	{
		*(Address + i) = valve_hook_bytes_[i];
	}
	VirtualProtect(Address, op_patch_lenght, dwOldProtect, &dwBkup);						//Revert permission change...

	return ret;
}


