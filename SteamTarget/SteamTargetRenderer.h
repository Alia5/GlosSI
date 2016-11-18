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

