#include "SteamTarget.h"
#include <Windows.h>
#include <QTimer>
#include <QSettings>
#include <qmessagebox.h>
#include <iostream>

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
	if (enable_controllers_)
		controller_thread_.run();
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
	controller_thread_.stop();
	target_overlay_.stop();
}

void SteamTarget::read_ini()
{
	if (arguments().size() == 1)
	{
		QMessageBox::warning(nullptr, "Error", "Target configuration file must be specified! Using default Values!");
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


