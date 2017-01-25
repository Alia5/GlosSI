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
#include <Xinput.h>
#include <ViGEmUM.h>

#include <Windows.h>

#include <SFML\System.hpp>

#include <iostream>

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

	bool bShouldRun = false;

	int iRealControllers = 0;
	int iTotalControllers = 0;
	int iVirtualControllers = 0;

	static ULONG ulTargetSerials[XUSER_MAX_COUNT];
	VIGEM_TARGET vtX360[XUSER_MAX_COUNT];
	XINPUT_STATE xsState[XUSER_MAX_COUNT];
	XUSB_REPORT xrReport[XUSER_MAX_COUNT];

	std::thread controllerThread;

	sf::Clock sfClock;
	int tickTime = 0;
	int delay = 1000000 / 200;

	void controllerLoop();

	int getRealControllers();

	static void controllerCallback(VIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber);

};

