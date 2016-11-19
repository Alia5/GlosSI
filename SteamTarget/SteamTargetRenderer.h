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
#pragma once

#include <Windows.h>
#include <dwmapi.h>

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

#include "VirtualControllerThread.h"

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

	VirtualControllerThread controllerThread;

	bool bEnableControllers = true;

	void RunSfWindowLoop();
	void makeSfWindowTransparent(sf::RenderWindow& window);
	void drawDebugEdges();

	void openUserWindow();

private slots:
	void userWindowFinished();
	void launchApp();
	void readChildProcess();
};

