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

#include "../common/common_hookfuns.h"
#include "../common/Injector.h"
#include "../common/process_alive.h"

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


SteamTarget::SteamTarget(int& argc, char** argv) : QApplication(argc, argv)
{
}

void SteamTarget::init()
{
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
	SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(ConsoleCtrlCallback), true);
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	read_ini();
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	target_overlay_.init(!enable_overlay_);
	initOverlayEvents();
	if (enable_controllers_)
		controller_thread_.run();
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

void SteamTarget::launchWatchdog() const
{
	const QString watchDogPath = QDir::toNativeSeparators(applicationDirPath()) + "\\GloSC_Watchdog.exe";
	QProcess::startDetached("explorer.exe", QStringList() << watchDogPath);
}

void SteamTarget::launchApplication()
{
	if (!launch_uwp_)
	{
		// To get our launched application not get hooked by Steam, we have to launch through Windows explorer
		// To use arguments, launching using explorer, we have to use a batch file...
		const QString batchContents = '\"' + QDir::toNativeSeparators(QString::fromStdString(launch_app_path_)) 
									+ '\"' + " " + QString::fromStdString(launch_app_args_);

		QFile file("launchApp.bat");
		if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
			QTextStream stream(&file);
			stream << "@Echo off\n" << batchContents;
			file.close();

			const QString launchPath = QDir::toNativeSeparators(applicationDirPath()) + "\\launchApp.bat";
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



