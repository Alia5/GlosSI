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
#include <QBuffer>
#include <qprocess.h>


class GloSC_GameLauncher : public QMainWindow
{
    Q_OBJECT

public:
    GloSC_GameLauncher(QWidget *parent = Q_NULLPTR);

public slots:
	void isAboutToBeKilled();

private:
    Ui::GloSC_GameLauncherClass ui;

	QSharedMemory sharedMemInstance;
	QTimer updateTimer;
	QStringList stringListFromShared;

	qint64 pid = NULL;

	void launchGameIfRequired();

	HRESULT LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId);

	bool IsProcessRunning(DWORD pid)
	{
		HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
		DWORD ret = WaitForSingleObject(process, 1);
		CloseHandle(process);
		return ret == WAIT_TIMEOUT;
	}

private slots:
	void checkSharedMem();

};
