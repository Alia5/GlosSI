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

#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <vector>

#include <Windows.h>
#include <psapi.h>

#include <SFML/System.hpp>

#include <Xinput.h>
#include <ViGEmUM.h>

class VirtualControllerThread
{
public:
	VirtualControllerThread();
	~VirtualControllerThread();

	void run();
	void stop();

	void resetControllers();

	bool isRunning();

private:

	std::atomic<bool> bShouldRun = false;


	typedef DWORD(WINAPI* XInputGetState_t)(DWORD dwUserIndex, XINPUT_STATE* pState);

	uint8_t valveHookBytes[5];
	uint8_t realBytes[5] = {0x48, 0x89, 0x5C, 0x24, 0x08};
	//uint8_t realBytes[5] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x90 };

	XInputGetState_t x_get_state = &XInputGetState;

	XInputGetState_t realXGetState = nullptr;

	//int iRealControllers = 0;
	//int iTotalControllers = 0;
	int iVirtualControllers = 0;
	int controllerCount = 0;

	bool checkedControllers = false;

	//static std::vector<ULONG> ulTargetSerials;

	VIGEM_TARGET vtX360[XUSER_MAX_COUNT];
	XINPUT_STATE xsState[XUSER_MAX_COUNT];

	std::thread controllerThread;

	sf::Clock sfClock;
	int tickTime = 0;
	int delay = 1000000 / 200;

	void controllerLoop();

	int getRealControllers();

	static void controllerCallback(VIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber);

	static DWORD XInputGetStateWrapper(DWORD dwUserIndex, XINPUT_STATE* pState);

	DWORD callRealXinputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);

};

