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

/*

SteamTarget - Does most of GloSCs heavy lifting.

*/

#pragma once
#include "TargetOverlay.h"
#include "VirtualControllerThread.h"

#include <QApplication>
#include <QTimer>

#include <functional>

#include <Windows.h>
#include <dwmapi.h>




class SteamTarget : public QApplication
{
	Q_OBJECT

public:
	SteamTarget(int& argc, char** argv);
	SteamTarget(const SteamTarget& other) = delete;
	SteamTarget(SteamTarget&& other) noexcept = delete;
	SteamTarget& operator=(const SteamTarget& other) = delete;
	SteamTarget& operator=(SteamTarget&& other) noexcept = delete;
	~SteamTarget() = default;

	void init();
	static BOOL WINAPI ConsoleCtrlCallback(DWORD dwCtrlType);

public slots:
	void onAboutToQuit();


private:
	void read_ini();
	void initOverlayEvents();

	void launchWatchdog() const;
	void launchApplication();

	static HRESULT LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId);

	TargetOverlay target_overlay_;
	VirtualControllerThread controller_thread_;

	//Settings from .ini file
	bool hook_steam_ = true;
	bool enable_overlay_ = true;
	bool enable_controllers_ = true;
	bool use_desktop_conf_ = false;
	bool launch_game_ = true;
	bool close_launched_done_ = true;
	bool launch_uwp_ = true;
	std::string launch_app_path_ = "Microsoft.ApolloBaseGame_8wekyb3d8bbwe!forzamotorsport7";
	std::string launch_app_args_ = "";

	QTimer launch_check_timer_;
	


	//Hooking stuff...
#ifdef _AMD64_
	const std::string overlay_module_name_ = "GameOverlayRenderer64.dll";
	const std::string overlay_open_func_sig_ = "TODO";
	const std::string overlay_open_func_mask_= "TODO";
#else
	const std::string overlay_module_name_ = "GameOverlayRenderer.dll";

	const char* overlay_open_func_sig_
		= "\x56\xC6\x46\x28\x01";
	const std::string overlay_open_func_mask_
		= "xxxxx";

	const char* overlay_closed_func_sig_
		= "\xC7\x46\x24\x00\x00\x00\x00\xC6\x46\x28\x00";
	const std::string overlay_closed_func_mask_
		= "xxxxxxxxxxx";

#endif


};

