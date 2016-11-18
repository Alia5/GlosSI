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

	void launchGameIfRequired();

	HRESULT LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId);


private slots:
	void checkSharedMem();

};
