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
#include <dwmapi.h>
#include <Xinput.h>
#include <ViGEmUM.h>

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>

#include <iostream>
#include <thread>

#include <QTimer>
#include <QProcess>
#include <QBuffer>
#include <QDatastream>
#include <QSharedmemory>
#include <QSettings>

class SteamTargetRenderer : public QObject
{
	Q_OBJECT

public:
	SteamTargetRenderer();
	~SteamTargetRenderer();

	void run();

private:
	bool bRunLoop = true;

	bool bDrawDebugEdges = false; //TODO: init from .ini ; make default false
	bool bDrawOverlay = true;
	bool bVsync = false;
	bool bPauseControllers = false;
	int iRefreshRate = 60;
	sf::CircleShape sfCshape;
	sf::RenderWindow sfWindow;

	QProcess *qpUserWindow;

	std::thread renderThread;

	bool bShowDebugConsole = true;
	HWND consoleHwnd;

	HMODULE hmodGameOverlayRenderer;
	uint64_t *overlayPtr = NULL;
	HWND hwForeGroundWindow = NULL;
	bool bNeedFocusSwitch = false;
	void getSteamOverlay();


	int bEnableControllers;
	int iRealControllers = 0;
	int iTotalControllers = 0;
	int iVirtualControllers = 0;

	static ULONG ulTargetSerials[XUSER_MAX_COUNT];
	VIGEM_TARGET vtX360[XUSER_MAX_COUNT];
	XINPUT_STATE xsState[XUSER_MAX_COUNT];
	XUSB_REPORT xrReport[XUSER_MAX_COUNT];

	int getRealControllers();

	void RunSfWindowLoop();
	static void controllerCallback(VIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber);
	void makeSfWindowTransparent(sf::RenderWindow& window);
	void drawDebugEdges();

	void openUserWindow();

private slots:
	void userWindowFinished();
	void launchApp();
	void readChildProcess();
};

