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

#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <vector>

#include <Windows.h>
#include <Psapi.h>

#include <SFML/System.hpp>

#include <Xinput.h>
#include <ViGEmClient.h>
#include <VersionHelpers.h>

class VirtualControllerThread
{
public:
	VirtualControllerThread();
	VirtualControllerThread(const VirtualControllerThread& other) = delete;
	VirtualControllerThread(VirtualControllerThread&& other) noexcept = delete;
	VirtualControllerThread& operator=(const VirtualControllerThread& other) = delete;
	VirtualControllerThread& operator=(VirtualControllerThread&& other) noexcept = delete;
	~VirtualControllerThread();

	void run();
	void stop();

	bool isRunning() const;

private:

	std::atomic<bool> b_should_run_ = false;
	typedef DWORD(WINAPI* XInputGetState_t)(DWORD dwUserIndex, XINPUT_STATE* pState);

	static const uint8_t op_patch_lenght = 5;
	uint8_t valve_hook_bytes_[5]{};

	bool seven_ = false;

#ifdef _AMD64_
	static constexpr const uint8_t realBytes[5] = {0x48, 0x89, 0x5C, 0x24, 0x08};
#else
	static constexpr const uint8_t real_bytes[5] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC };
#endif
	//uint8_t realBytes[5] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x90 };

	int controller_count_ = 0;
	XInputGetState_t x_get_state_ = nullptr;

	PVIGEM_CLIENT driver_;
	PVIGEM_TARGET vt_x360_[XUSER_MAX_COUNT]{};

	std::thread controller_thread_;

	sf::Clock sf_clock_;
	int tick_time_ = 0;
	constexpr static int delay = 1000000 / 200;

	void controllerLoop();

	static void __RPC_CALLEE controllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber);

	static DWORD XInputGetStateWrapper(DWORD dwUserIndex, XINPUT_STATE* pState); //Easier to find in x64dbg...

	DWORD callRealXinputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
};

