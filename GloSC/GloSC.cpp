/*
Copyright 2016 Peter Repukat - FlatspotSoftware

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
#include "GloSC.h"

GloSC::GloSC(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	updateEntryList();
	updateTargetsToNewVersion();

	//Launch the gamelauncher here, just to be safe
	//Don't need to check if the process already exists as it does it itself
	QProcess proc;
	proc.startDetached("GloSC_Gamelauncher.exe", QStringList(), QDir::toNativeSeparators(QApplication::applicationDirPath()), nullptr);

}

void GloSC::updateEntryList()
{
	ui.lwInstances->clear();

	QDir dir(".\\");
	QStringList dirNames = dir.entryList(QDir::Dirs);

	for (auto &dirName : dirNames)
	{
		if (dirName != "." && dirName != ".." && dirName != "platforms")
			ui.lwInstances->addItem(dirName);
	}


}

void GloSC::writeIni(QString entryName)
{
	QSettings settings(entryName + "\\TargetConfig.ini", QSettings::IniFormat);

	settings.beginGroup("BaseConf");

	settings.setValue("bEnableOverlay", 0 + ui.cbOverlay->isChecked());
	settings.setValue("bEnableControllers", 0 + ui.cbControllers->isChecked());
	settings.setValue("bUseDesktopConfig", 0 + ui.cbUseDesktop->isChecked());
	settings.setValue("bHookSteam", 0 + ui.cbHookSteam->isChecked());
	settings.setValue("version", GLOSC_VERSION);

	settings.endGroup();


	settings.beginGroup("LaunchGame");

	settings.setValue("bLaunchGame", 0 + ui.cbLaunchGame->isChecked());
	if (ui.cbLaunchGame->isChecked())
	{
		settings.setValue("Path", ui.lePath->text());
		if (ui.lePath->text().contains("\\") || ui.lePath->text().contains("/"))
		{
			settings.setValue("Type", "Win32");
		} else {
			settings.setValue("Type", "UWP");
		}
	}

	settings.setValue("bCloseWhenDone", 0 + ui.cbCloseWhenDone->isChecked());

	settings.endGroup();

}

void GloSC::updateTargetsToNewVersion()
{
	//incredible lazy way to update to this next version but eh...
	for (int i = 0; i < ui.lwInstances->count(); i++)
	{
		on_lwInstances_currentRowChanged(i);
		QString name = ui.leName->text();

		QSettings settings(name + "\\TargetConfig.ini", QSettings::IniFormat);
		settings.beginGroup("BaseConf");
		unsigned int version = settings.value("version").toInt();
		settings.endGroup();

		if (version < GLOSC_VERSION)
		{
			QFile file(name + "\\" + name + ".exe");
			file.remove();
			on_pbSave_clicked();
		}
	}
}


void GloSC::animate(int to)
{
	if (to == width())
		return;
	QPropertyAnimation* anim = new QPropertyAnimation(this, "size");
	if (to > width())
	{
		anim->setEasingCurve(QEasingCurve::InOutExpo);
		connect(anim, &QPropertyAnimation::finished, this, [this, to]()
		{
			this->setMinimumWidth(to);
		});
		this->setMaximumWidth(to);

		QPropertyAnimation* buttonAnim = new QPropertyAnimation(ui.pbCreateNew, "size");
		buttonAnim->setEasingCurve(QEasingCurve::InOutExpo);
		buttonAnim->setDuration(360);
		buttonAnim->setStartValue(QSize(ui.pbCreateNew->width(), ui.pbCreateNew->height()));
		buttonAnim->setEndValue(QSize(wide_x_create, ui.pbCreateNew->height()));
		buttonAnim->start(QPropertyAnimation::DeleteWhenStopped);

	}
	else
	{
		anim->setEasingCurve(QEasingCurve::InExpo);
		connect(anim, &QPropertyAnimation::finished, this, [this, to]()
		{
			this->setMaximumWidth(to);
		});
		this->setMinimumWidth(to);

		QPropertyAnimation* buttonAnim = new QPropertyAnimation(ui.pbCreateNew, "size");
		buttonAnim->setEasingCurve(QEasingCurve::InExpo);
		buttonAnim->setDuration(360);
		buttonAnim->setStartValue(QSize(ui.pbCreateNew->width(), ui.pbCreateNew->height()));
		buttonAnim->setEndValue(QSize(small_x_create, ui.pbCreateNew->height()));
		buttonAnim->start(QPropertyAnimation::DeleteWhenStopped);

	}
	anim->setDuration(360);
	anim->setStartValue(QSize(this->width(), this->height()));
	anim->setEndValue(QSize(to, this->height()));
	anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void GloSC::on_cbUseDesktop_toggled(bool checked)
{
	ui.cbHookSteam->setEnabled(!checked);
	ui.cbHookSteam->setChecked(!checked);
}

void GloSC::on_pbCreateNew_clicked()
{
	ui.leName->setText("");

	ui.cbOverlay->setChecked(true);
	ui.cbControllers->setChecked(true);
	ui.cbHookSteam->setChecked(true);

	ui.cbLaunchGame->setChecked(false);
	ui.lePath->setText("");
	ui.cbCloseWhenDone->setChecked(false);

	animate(wide_x);
}

void GloSC::on_pbSave_clicked()
{
	QString name = ui.leName->text();
	name.remove("\\");
	name.remove("/");
	name.remove(":");
	name.remove(".");

	QString temp = name;
	if (temp.remove(" ") == "")
		return;

	QDir dir(name);
	if (!dir.exists())
		dir.mkdir(".");

#ifdef NDEBUG
	QString fileNames[] = {
		"Qt5Core.dll",
		"Qt5Gui.dll",
		"Qt5Widgets.dll",
		"sfml-system-2.dll",
		"sfml-window-2.dll",
		"sfml-graphics-2.dll",
		"ViGEmUM.dll",
		"TargetConfig.ini" };
#else
	QString fileNames[] = {
		"Qt5Cored.dll",
		"Qt5Guid.dll",
		"Qt5Widgetsd.dll",
		"sfml-system-d-2.dll",
		"sfml-window-d-2.dll",
		"sfml-graphics-d-2.dll",
		"ViGEmUM.dll",
		"TargetConfig.ini"
};
#endif

	for (auto &fileName : fileNames)
	{
		QFile::copy(fileName, dir.path() + "\\" + fileName);
	}
	QDir platformdir(name + "\\platforms");
	if (!platformdir.exists())
		platformdir.mkdir(".");

	QFile file(dir.path() + "\\" + name + ".exe");
	file.remove();


	QFile::copy("platforms\\qwindows.dll", dir.path() + "\\" + "platforms\\qwindows.dll");
	QFile::copy("SteamTarget.exe", dir.path() + "\\" + name + ".exe");

	writeIni(name);

	updateEntryList();

	animate(small_x);
}


void GloSC::on_pbDelete_clicked()
{
	QString name = ui.leName->text();
	name.remove("\\");
	name.remove("/");
	name.remove(":");
	name.remove(".");

	QString temp = name;
	if (temp.remove(" ") == "")
		return;

	QDir dir(name);
	if (dir.exists())
	{
		dir.removeRecursively();
	}
	updateEntryList();

	animate(small_x);
}

void GloSC::on_pbAddToSteam_clicked()
{
	if (ui.lwInstances->count() <= 0)
	{
		QMessageBox::information(this, "GloSC", "No shortcuts! Create some shortcuts first for them to be added to Steam!", QMessageBox::Ok);
		return;
	}

	QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Valve\\Steam", QSettings::NativeFormat);
	QString steamPath = settings.value("SteamPath").toString();
	QString activeUser = settings.value("ActiveProcess/ActiveUser").toString();

	QFile shortcutsFile(steamPath + "/userdata/" + activeUser + "/config/shortcuts.vdf");

	if (!shortcutsFile.exists())
	{
		QMessageBox::information(this, "GloSC", "Couldn't detect Steam shortcuts file!\nSteam must be running for it to be detected", QMessageBox::Ok);
		return;
	}
	if (!shortcutsFile.open(QFile::ReadWrite))
	{
		QMessageBox::information(this, "GloSC", "Couldn't open Steam shortcuts file!", QMessageBox::Ok);
		return;
	}

	//just detect already present paths the easy way and hardcode the actual shortcut structure
	//will prob. come back to bite me, but for now it should be enough
	QByteArray shortcutsFileBytes = shortcutsFile.readAll();

	//get shortcutcount
	QByteArray temp = shortcutsFileBytes;
	temp.chop(9); //chop off last "tags"
	temp = temp.mid(temp.lastIndexOf("tags") + 8, temp.size() - 1);
	int shortcutCount = QString(temp).toInt();

	QString itemName;
	QString appDir = QDir::toNativeSeparators(QCoreApplication::applicationFilePath().mid(0, QCoreApplication::applicationFilePath().lastIndexOf("/")));
	for (int i = 0; i < ui.lwInstances->count(); i++)
	{
		itemName = ui.lwInstances->item(i)->text();
		if (!shortcutsFileBytes.contains(QString(appDir + "\\" + itemName + "\\" + itemName + ".exe").toStdString().c_str()))
		{
			shortcutsFileBytes.chop(2); //chop of end bytes
			shortcutCount++;

			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append(QString::number(shortcutCount));
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x01');
			shortcutsFileBytes.append("appname");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append(itemName);
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x01');
			shortcutsFileBytes.append("exe");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append(QString("\"" + appDir + "\\" + itemName + "\\" + itemName + ".exe\""));
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x01');
			shortcutsFileBytes.append("StartDir");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append(QString("\"" + appDir + "\\" + itemName + "\\" + "\""));
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x01');
			shortcutsFileBytes.append("icon");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x01');
			shortcutsFileBytes.append("ShortcutPath");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x02');
			shortcutsFileBytes.append("IsHidden");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x02');
			shortcutsFileBytes.append("AllowDesktopConfig");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x01');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x02');
			shortcutsFileBytes.append("OpenVR");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append("tags");
			shortcutsFileBytes.append('\x00');
			shortcutsFileBytes.append('\x08');
			shortcutsFileBytes.append('\x08');

			//append chopped of bytes
			shortcutsFileBytes.append('\x08');
			shortcutsFileBytes.append('\x08');
		}
	}

	shortcutsFile.close();
	if (!shortcutsFile.open(QFile::ReadWrite | QIODevice::Truncate)) 
	{
		QMessageBox::information(this, "GloSC", "Couldn't open Steam shortcuts file!", QMessageBox::Ok);
		return;
	}

	shortcutsFile.write(shortcutsFileBytes);

	shortcutsFile.close();
	QMessageBox::information(this, "GloSC", "Shortcuts were added! Restart Steam for changes to take effect!", QMessageBox::Ok);

	animate(small_x);
}

void GloSC::on_pbSearchPath_clicked()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Select Game", "", "*.exe");
	ui.lePath->setText(filePath);
	if (filePath.length() > 0)
	{
		QString name;
		if (filePath.contains("\\"))
			name = filePath.mid(filePath.lastIndexOf("\\") + 1, -1);
		else
			name = filePath.mid(filePath.lastIndexOf("/") + 1, -1);
		name.chop(4);
		ui.leName->setText(name);
	}
}

void GloSC::on_pbUWP_clicked()
{
	QSettings *settings = new QSettings("HKEY_CLASSES_ROOT", QSettings::NativeFormat);

	QStringList childs = settings->childGroups();
	QStringList packages;

	for (auto& child : childs)
	{
		if (child.indexOf("AppX") == 0)
		{
			packages << child;
		}
	}

	delete settings;


	QList<UWPPair> pairs;

	QString AppName;
	QString AppUMId;

	QStringList AppNames;
	QStringList AppUMIds;


	for (auto &package : packages)
	{
		settings = new QSettings("HKEY_CLASSES_ROOT\\"+package, QSettings::NativeFormat);

		AppName = settings->value("Application/ApplicationName").toString();
		AppUMId = settings->value("Application/AppUserModelID").toString();
		if (!AppNames.contains(AppName) && !AppUMIds.contains(AppUMId) && AppUMId.size() > 0)
		{

			AppNames << AppName;
			AppUMIds << AppUMId;

			if (AppName.size() == 0)
			{
				AppName = "Unknown";
			} else if (AppName.at(0) == '@') {
				QString packageName = AppName.mid(AppName.indexOf('{') + 1, AppName.size() -1);
				packageName = packageName.mid(0, packageName.indexOf('?'));
				QStringList cachedNameChildGroups;
				QSettings settings("HKEY_CLASSES_ROOT\\Local Settings\\MrtCache", QSettings::NativeFormat);

				cachedNameChildGroups = settings.childGroups();

				for (auto &childGroup : cachedNameChildGroups)
				{
					
					if (childGroup.contains(packageName))
					{
						QSettings settings("HKEY_CLASSES_ROOT\\Local Settings\\MrtCache\\"+ childGroup, QSettings::NativeFormat);

						QStringList allKeys = settings.allKeys();

						AppName.replace("/", "\\");
						for (auto &key : allKeys)
						{
							if (key.contains(AppName))
							{
								AppName = settings.value(key).toString();
								break;
							}
						}

						break;
					}
				}
				if (AppName.at(0) == '@') {
					AppName = "Unknown";
				}
			}

			UWPPair uwpPair = {
				AppName,
				AppUMId,
			};

			pairs.push_back(uwpPair);

		}
		delete settings;
	}

	uwpPairs = pairs;


	UWPSelectDialog dialog(this);
	dialog.setUWPList(uwpPairs);
	int selection = dialog.exec();

	if (selection > -1)
	{
		ui.lePath->setText(uwpPairs.at(selection).AppUMId);
		ui.leName->setText(uwpPairs.at(selection).AppName);
	}

}

void GloSC::on_lwInstances_currentRowChanged(int row)
{
	if (row < 0)
		return;
	QString entryName = ui.lwInstances->item(row)->text();
	ui.leName->setText(entryName);

	QSettings settings(entryName + "\\TargetConfig.ini", QSettings::IniFormat);

	settings.beginGroup("BaseConf");

	ui.cbOverlay->setChecked(settings.value("bEnableOverlay").toBool());
	ui.cbControllers->setChecked(settings.value("bEnableControllers").toBool());
	ui.cbUseDesktop->setChecked(settings.value("bUseDesktopConfig").toBool());
	if (ui.cbUseDesktop->isChecked())
	{
		ui.cbHookSteam->setChecked(false);
		ui.cbHookSteam->setEnabled(false);
	}
	else
	{
		ui.cbHookSteam->setEnabled(true);
		ui.cbHookSteam->setChecked(settings.value("bHookSteam").toBool());
	}

	settings.endGroup();


	settings.beginGroup("LaunchGame");

	ui.cbLaunchGame->setChecked(settings.value("bLaunchGame").toBool());
	if (ui.cbLaunchGame->isChecked())
	{
		ui.lePath->setText(settings.value("Path").toString());
	}
	ui.cbCloseWhenDone->setChecked(settings.value("bCloseWhenDone").toBool());

	settings.endGroup();

}

void GloSC::on_lwInstances_itemSelectionChanged()
{
	if (width() != wide_x)
	{
		animate(wide_x);
	} else {
		//ui.configBox->setGraphicsEffect(&opEff);
		//QPropertyAnimation* anim = new QPropertyAnimation(&opEff, "opacity");
		//anim->setEasingCurve(QEasingCurve::OutExpo);
		//anim->setDuration(160);
		//anim->setStartValue(1.f);
		//anim->setEndValue(0.f);
		//connect(anim, &QPropertyAnimation::finished, this, [this]()
		//{
		//	QPropertyAnimation* anim2 = new QPropertyAnimation(&opEff, "opacity");
		//	anim2->setEasingCurve(QEasingCurve::InExpo);
		//	anim2->setDuration(160);
		//	anim2->setStartValue(0.f);
		//	anim2->setEndValue(1.f);
		//	anim2->start(QPropertyAnimation::DeleteWhenStopped);
		//});
		//anim->start(QPropertyAnimation::DeleteWhenStopped);
	}
}
