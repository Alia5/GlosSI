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
#pragma once

#include <Windows.h>
#include <atlbase.h>
#include <Shobjidl.h>
#include <appmodel.h>
#include <AppxPackaging.h>
#include <psapi.h>

#include <QtWidgets/QMainWindow>
#include "ui_GloSC_GameLauncher.h"


#include <QTimer>
#include <QSharedmemory>
#include <QRegularExpression>
#include <QBuffer>
#include <qprocess.h>

#include <qmessagebox.h>


class GloSC_GameLauncher : public QMainWindow
{
	Q_OBJECT

public:
	GloSC_GameLauncher(QWidget *parent = Q_NULLPTR);

public slots:
	void isAboutToBeKilled();

private:
	Ui::GloSC_GameLauncherClass ui;

	const QString LaunchGame = "LaunchGame";
	const QString LGT_UWP = "UWP";
	const QString LGT_Win32 = "Win32";
	const QString LaunchedProcessFinished = "LaunchedProcessFinished";
	const QString IsSteamHooked = "IsSteamHooked";
	const QStringList defaultSharedMemData = QStringList()
		<< LaunchGame
		<< ""
		<< ""
		<< ""
		<< LaunchedProcessFinished
		<< "0"
		<< IsSteamHooked
		<< "-1";

	

	QSharedMemory sharedMemInstance;
	QTimer updateTimer;

	qint64 pid = NULL;

	bool bHookedSteam = false;

	void launchGame(QString type, QString path, QStringList args);

	HRESULT LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId);

	bool IsProcessRunning(DWORD pid);

	void unhookBindings();

private slots:
	void checkSharedMem();

};
