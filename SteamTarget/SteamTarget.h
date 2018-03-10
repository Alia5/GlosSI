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
#include <QApplication>
#include <thread>
#include <Windows.h>
#include <dwmapi.h>
#include "TargetOverlay.h"
#include "VirtualControllerThread.h"


class SteamTarget : public QApplication
{
	Q_OBJECT

public:
	SteamTarget(int& argc, char** argv);
	~SteamTarget() = default;

	void init();
	static BOOL WINAPI ConsoleCtrlCallback(DWORD dwCtrlType);

public slots:
	void onAboutToQuit();


private:
	void read_ini();

	TargetOverlay target_overlay_;
	VirtualControllerThread controller_thread_;

	//Settings from .ini file
	bool hook_steam_ = true;
	bool enable_overlay_ = true;
	bool enable_controllers_ = true;
	bool use_desktop_conf_ = false;
	bool launch_game_ = false;
	bool close_launched_done_ = false;
	bool launch_uwp_ = false;
	std::string launch_app_path_ = "";
	std::string launch_app_args_ = "";

};

