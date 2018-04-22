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

#include "OverlayHookFunction.h"
#include "ForegroundWindowHook.h"

#include "../common/common_hookfuns.h"
#include "../common/Injector.h"
#include "../common/process_alive.h"

#define LOGURU_IMPLEMENTATION 1
#include "../common/loguru.hpp"

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QSettings>
#include <qmessagebox.h>
#include <iostream>

#include <Windows.h>
#include <atlbase.h>
#include <Shobjidl.h>
#include <qstandardpaths.h>


SteamTarget::SteamTarget(int& argc, char** argv) : QApplication(argc, argv)
{
	loguru::init(argc, argv);
}

void SteamTarget::init()
{
	loguru::add_file(QString(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + "/last.log").toStdString().c_str(),
		loguru::Truncate, loguru::Verbosity_INFO);
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
	SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(ConsoleCtrlCallback), true);
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	readIni();
	target_overlay_.init(!enable_overlay_, enable_overlay_only_config_, max_fps_);
	initOverlayEvents();
	if (!use_desktop_conf_)
		fgwinhook::patchForegroundWindow();
	controller_thread_ = std::make_unique<VirtualControllerThread>(update_rate_);
	if (enable_controllers_)
		controller_thread_->run();
	if (hook_steam_ && !use_desktop_conf_)
		Injector::hookSteam();
	launchWatchdog();
	if (launch_game_)
		launchApplication();

	sys_tray_icon_.setIcon(QIcon(":/SteamTarget/Resources/GloSC_Icon.png"));
	tray_icon_menu_.addAction("Quit");
	sys_tray_icon_.setContextMenu(&tray_icon_menu_);

	connect(&sys_tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	connect(*tray_icon_menu_.actions().begin(), SIGNAL(triggered()), this, SLOT(quit()));

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

	controller_thread_->stop();
	target_overlay_.stop();
}

void SteamTarget::readIni()
{
	if (arguments().size() == 1)
	{
		LOG_F(WARNING, "Target configuration file must be specified! Using default Values!");
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
			if (childkey == "bEnableOverlayOnlyConfig") {
				enable_overlay_only_config_ = settings.value(childkey).toBool();
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
			else if (childkey == "iUpdateRate") {
				bool isInt = false;
				update_rate_ = settings.value(childkey).toInt(&isInt);
				if (!isInt || update_rate_ < 0)
					update_rate_ = 5000;
			}
			else if (childkey == "iRefreshRate") {
				bool isInt = false;
				max_fps_ = settings.value(childkey).toInt(&isInt);
				if (!isInt || max_fps_ < 0)
					max_fps_ = 60;
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
			else if (childkey == "Args") {
				launch_app_args_ = settings.value(childkey).toString().toStdString();
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
		const DWORD addressOpen = hook_commons::FindPattern(overlay_module_name,
			overlay_open_func_sig,
			overlay_open_func_mask);

		if (addressOpen != 0)
		{
			DWORD addressClosed = 0;

			for (DWORD i = 0; i < 1024; i++)	//search next signature relativ to "addressOpened"
			{
				bool found = true;
				for (DWORD j = 0; j < std::string(overlay_closed_func_mask).length(); j++)
					found &=
						overlay_closed_func_mask[j] == '?' || 
					overlay_closed_func_sig[j] == *reinterpret_cast<char*>(addressOpen + j + i);

				if (found)
				{
					addressClosed = addressOpen + i;
					break;
				}
			}

			if (addressClosed != 0)
			{
				overlay_hook::JMPBackOpen = addressOpen + std::string(overlay_open_func_mask).length();
				overlay_hook::JMPBackClosed = addressClosed + std::string(overlay_closed_func_mask).length();
				overlay_hook::target_overlay = &target_overlay_;

				hook_commons::PlaceJMP(reinterpret_cast<BYTE*>(addressOpen),
					reinterpret_cast<DWORD>(overlay_hook::overlay_opend_hookFN), std::string(overlay_open_func_mask).length());

				hook_commons::PlaceJMP(reinterpret_cast<BYTE*>(addressClosed),
					reinterpret_cast<DWORD>(overlay_hook::overlay_closed_hookFN), std::string(overlay_closed_func_mask).length());
			} else {
				LOG_F(WARNING, "Failed to find overlayClosed signature!");
			}
		} else {
			LOG_F(WARNING, "Failed to find overlayOpened signature!");
		}
}

void SteamTarget::launchWatchdog() const
{
	const QString watchDogPath = QDir::toNativeSeparators(applicationDirPath()) + "\\GloSC_Watchdog.exe";
	if(QProcess::startDetached("explorer.exe", QStringList() << watchDogPath))
		LOG_F(INFO, "Launched Watchdog");
	else
		LOG_F(WARNING, "Failed to launch Watchdog!");
}

void SteamTarget::launchApplication()
{
	if (!launch_uwp_)
	{
		// To get our launched application not get hooked by Steam, we have to launch through Windows explorer
		// To use arguments, launching using explorer, we have to use a batch file...

		QString programPath = QDir::toNativeSeparators(QString::fromStdString(launch_app_path_));

		const QString batchContents = 
			"cd /D \"" + programPath.mid(0, programPath.lastIndexOf("\\")) + "\"\n"
				+ '\"' + programPath.mid(programPath.lastIndexOf("\\")+1)
				+ '\"' + " " + QString::fromStdString(launch_app_args_);

		QFile file(QString(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + "/launchApp.bat"));
		if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
			QTextStream stream(&file);
			stream << "@Echo off\n" << batchContents;
			file.close();

			const QString launchPath = QDir::toNativeSeparators(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0)) + "\\launchApp.bat";
			QProcess::startDetached("explorer.exe", QStringList() << launchPath);

			if (close_launched_done_)
			{
				const QString appName = QDir::toNativeSeparators(QString::fromStdString(launch_app_path_)).split('\\').last();
				connect(&launch_check_timer_, &QTimer::timeout, [appName]()
				{
					if (!process_alive::IsProcessRunning(appName.toStdWString().c_str()))
						SteamTarget::quit();
				});
				QTimer::singleShot(10000, this, [this]()
				{
					launch_check_timer_.start(1000);
				});
			}

		}
	}
	else
	{
		// We don't need such bullshit explorer tricks when dealing with UWP, as Valve still hasn't figured out how to hook them.
		// Or they just don't wan't to
		// If you're interested in how to hook UWP: https://behind.flatspot.pictures/hacking-windows-universal-apps-uwp/

		DWORD pid = 0;
		const HRESULT hr = CoInitialize(nullptr);
		std::wstring appUMId = QString::fromStdString(launch_app_path_).toStdWString();
		if (SUCCEEDED(hr)) {
			const HRESULT result = LaunchUWPApp(appUMId.c_str(), &pid);
			if (SUCCEEDED(result))
			{
				if (close_launched_done_)
				{
					connect(&launch_check_timer_, &QTimer::timeout, [pid]()
					{
						if (!process_alive::IsProcessRunning(pid))
							SteamTarget::quit();
					});
					QTimer::singleShot(10000, this, [this]()
					{
						launch_check_timer_.start(1000);
					});
				}
			}
		}
		CoUninitialize();
	}


}

HRESULT SteamTarget::LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId)
{
	CComPtr<IApplicationActivationManager> spAppActivationManager;
	HRESULT result = E_INVALIDARG;
	// Initialize IApplicationActivationManager
	result = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_LOCAL_SERVER, IID_IApplicationActivationManager, (LPVOID*)&spAppActivationManager);

	if (!SUCCEEDED(result))
		return result;
	
	// This call ensures that the app is launched as the foreground window and sometimes may randomly fail...
	result = CoAllowSetForegroundWindow(spAppActivationManager, NULL);
	
	// Launch the app
	result = spAppActivationManager->ActivateApplication(packageFullName, NULL, AO_NONE, pdwProcessId);

	return result;
}



