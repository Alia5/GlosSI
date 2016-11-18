/*
Copyright 2016 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "SteamTargetUserWindow.h"

SteamTargetUserWindow::SteamTargetUserWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QSettings settings(".\\TargetConfig.ini", QSettings::IniFormat);
	settings.beginGroup("BaseConf");
	const QStringList childKeys = settings.childKeys();
	for (auto &childkey : childKeys)
	{
		if (childkey == "bShowDebugConsole") {
			ui.checkBox_showDebugConsole->setChecked(settings.value(childkey).toBool());
		}
		else if (childkey == "bEnableOverlay") {
			ui.checkBox_showOverlay->setChecked(settings.value(childkey).toBool());
		}
		else if (childkey == "bEnableControllers") {
			ui.checkBox_enableControllers->setChecked(settings.value(childkey).toBool());
		}
	}
	settings.endGroup();

	enableIPC = true;

}

void SteamTargetUserWindow::on_pushButton_resetControllers_clicked()
{
	if (!enableIPC)
		return;
	std::cout << "ResetControllers" << std::endl;

	QSettings settings(".\\TargetConfig.ini", QSettings::IniFormat);
	settings.beginGroup("BaseConf");
}

void SteamTargetUserWindow::on_checkBox_showOverlay_toggled()
{
	if (!enableIPC)
		return;
	std::cout << "ShowOverlay " << ui.checkBox_showOverlay->isChecked() << std::endl;
}

void SteamTargetUserWindow::on_checkBox_enableControllers_toggled()
{
	if (!enableIPC)
		return;
	std::cout << "EnableControllers " << ui.checkBox_enableControllers->isChecked() << std::endl;
}

void SteamTargetUserWindow::on_checkBox_showDebugConsole_toggled()
{
	if (!enableIPC)
		return;
	std::cout << "ShowConsole " << ui.checkBox_showDebugConsole->isChecked() << std::endl;
}
