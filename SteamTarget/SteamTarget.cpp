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
#include "SteamTarget.h"
#include <Windows.h>
#include <QTimer>
#include <QSettings>
#include <qmessagebox.h>
#include <iostream>

#include "../common/common_hookfuns.h"
#include "OverlayHookFunction.h"
#include "Injector.h"
#include <tlhelp32.h>

SteamTarget::SteamTarget(int& argc, char** argv) : QApplication(argc, argv)
{
}

void SteamTarget::init()
{
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
	SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(ConsoleCtrlCallback), true);
	read_ini();
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	target_overlay_.init(!enable_overlay_);
	initOverlayEvents();
	if (enable_controllers_)
		controller_thread_.run();
	if (hook_steam_ && !use_desktop_conf_)
		Injector::hookSteam();
}

BOOL SteamTarget::ConsoleCtrlCallback(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT || dwCtrlType == CTRL_BREAK_EVENT || dwCtrlType == CTRL_C_EVENT)
	{
		quit();
		return true;
	}
	return false;
}

void SteamTarget::onAboutToQuit()
{
	if (hook_steam_ && !use_desktop_conf_)
		Injector::unhookSteam();

	controller_thread_.stop();
	target_overlay_.stop();
}

void SteamTarget::read_ini()
{
	if (arguments().size() == 1)
	{
		QMessageBox::warning(nullptr, "GloSC", "Target configuration file must be specified! Using default Values!");
	}
	else {
		QSettings settings(arguments().at(1), QSettings::IniFormat);
		settings.beginGroup("BaseConf");
		QStringList childKeys = settings.childKeys();
		for (auto &childkey : childKeys)
		{
			if (childkey == "bEnableOverlay") {
				enable_overlay_ = settings.value(childkey).toBool();
			}
			else if (childkey == "bEnableControllers") {
				enable_controllers_ = settings.value(childkey).toBool();
			}
			else if (childkey == "bHookSteam") {
				hook_steam_ = settings.value(childkey).toBool();
			}
			else if (childkey == "bUseDesktopConfig") {
				use_desktop_conf_ = settings.value(childkey).toBool();
			}
		}
		settings.endGroup();
		settings.beginGroup("LaunchGame");
		childKeys = settings.childKeys();
		for (auto &childkey : childKeys)
		{
			if (childkey == "bLaunchGame") {
				launch_game_ = settings.value(childkey).toBool();
			}
			else if (childkey == "Path") {
				launch_app_path_ = settings.value(childkey).toString().toStdString();
			}
			else if (childkey == "Type") {
				launch_uwp_ = settings.value(childkey).toString() == QString("UWP");
			}
			else if (childkey == "bCloseWhenDone") {
				close_launched_done_ = settings.value(childkey).toBool();
			}
		}
		settings.endGroup();
	}
}

void SteamTarget::initOverlayEvents()
{
		//You hook into **MY** process? I'm ready to play your games, Valve! I'll hook back!!! 😅
		const DWORD addressOpen = hook_commons::FindPattern(overlay_module_name_.data(),
			overlay_open_func_sig_,
			overlay_open_func_mask_.data());

		if (addressOpen != 0)
		{
			DWORD addressClosed = 0;

			for (DWORD i = 0; i < 1024; i++)	//search next signature relativ to "addressOpened"
			{
				bool found = true;
				for (DWORD j = 0; j < overlay_closed_func_mask_.length(); j++)
					found &=
						overlay_closed_func_mask_[j] == '?' || 
					overlay_closed_func_sig_[j] == *reinterpret_cast<char*>(addressOpen + j + i);

				if (found)
				{
					addressClosed = addressOpen + i;
					break;
				}
			}

			if (addressClosed != 0)
			{
				overlay_hook::JMPBackOpen = addressOpen + overlay_open_func_mask_.length();
				overlay_hook::JMPBackClosed = addressClosed + overlay_closed_func_mask_.length();
				overlay_hook::target_overlay = &target_overlay_;

				hook_commons::PlaceJMP(reinterpret_cast<BYTE*>(addressOpen),
					reinterpret_cast<DWORD>(overlay_hook::overlay_opend_hookFN), overlay_open_func_mask_.length());

				hook_commons::PlaceJMP(reinterpret_cast<BYTE*>(addressClosed),
					reinterpret_cast<DWORD>(overlay_hook::overlay_closed_hookFN), overlay_closed_func_mask_.length());
			}
		}
}



