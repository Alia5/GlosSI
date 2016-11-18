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
