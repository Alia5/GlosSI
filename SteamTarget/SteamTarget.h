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
#include <QSystemTrayIcon>
#include <QMenu>

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
	void readIni();
	void initOverlayEvents();

	void launchWatchdog() const;
	void launchApplication();

	static HRESULT LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId);

	TargetOverlay target_overlay_;
	std::unique_ptr<VirtualControllerThread> controller_thread_;

	//Settings from .ini file
	bool hook_steam_ = false;
	bool enable_overlay_ = true;
	bool enable_overlay_only_config_ = false;
	bool enable_controllers_ = true;
	bool use_desktop_conf_ = false;
	bool launch_game_ = false;
	bool close_launched_done_ = false;
	bool launch_uwp_ = false;
	std::string launch_app_path_ = "";
	std::string launch_app_args_ = "";
	int update_rate_ = 5000;

	QTimer launch_check_timer_;
	
	QSystemTrayIcon sys_tray_icon_;
	QMenu tray_icon_menu_;


	//Hooking stuff...
#ifdef _AMD64_
	constexpr static const char* overlay_module_name = "GameOverlayRenderer64.dll";
	constexpr static const char* overlay_open_func_sig = "TODO";
	constexpr static const char* overlay_open_func_mask= "TODO";
#else
	constexpr static const char* overlay_module_name = "GameOverlayRenderer.dll";

	constexpr static const char* overlay_open_func_sig = "\x56\xC6\x46\x28\x01";
	constexpr static const char* overlay_open_func_mask = "xxxxx";

	constexpr static const char* overlay_closed_func_sig = "\xC7\x46\x24\x00\x00\x00\x00\xC6\x46\x28\x00";
	constexpr static const char* overlay_closed_func_mask = "xxxxxxxxxxx";

#endif


};

