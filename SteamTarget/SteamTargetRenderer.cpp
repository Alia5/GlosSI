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
#include "SteamTargetRenderer.h"



SteamTargetRenderer::SteamTargetRenderer()
{
	getSteamOverlay();

	openUserWindow();
#ifndef NDEBUG
	bDrawDebugEdges = true;
#endif // NDEBUG

	QSettings settings(".\\TargetConfig.ini", QSettings::IniFormat);
	settings.beginGroup("BaseConf");
	const QStringList childKeys = settings.childKeys();
	for (auto &childkey : childKeys)
	{
		if (childkey == "bDrawDebugEdges")
		{
			bDrawDebugEdges = settings.value(childkey).toBool();
		} else if (childkey == "bShowDebugConsole") {
			bShowDebugConsole = settings.value(childkey).toBool();
		} else if (childkey == "bEnableOverlay") {
			bDrawOverlay = settings.value(childkey).toBool();
		} else if (childkey == "bEnableControllers") {
			bEnableControllers = settings.value(childkey).toBool();
		} /*else if (childkey == "bEnableVsync") {
			bVsync = settings.value(childkey).toBool();
		} else if (childkey == "iRefreshRate") {
			iRefreshRate = settings.value(childkey).toInt();
		}*/
	}
	settings.endGroup();

	sfCshape = sf::CircleShape(100.f);
	sfCshape.setFillColor(sf::Color(128, 128, 128, 128));
	sfCshape.setOrigin(sf::Vector2f(100, 100));
	sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	sfWindow.create(sf::VideoMode(mode.width-16, mode.height-32), "OverlayWindow"); //Window is too large ; always 16 and 32 pixels?  - sf::Style::None breaks transparency!
	sfWindow.setVerticalSyncEnabled(bVsync);
	if (!bVsync)
		sfWindow.setFramerateLimit(iRefreshRate);
	sfWindow.setPosition(sf::Vector2i(0, 0));
	makeSfWindowTransparent(sfWindow);

	sfWindow.setActive(false);
	consoleHwnd = GetConsoleWindow(); //We need a console for a dirty hack to make sure we stay in game bindings - Also useful for debugging

	LONG_PTR style = GetWindowLongPtr(consoleHwnd, GWL_STYLE); 
	SetWindowLongPtr(consoleHwnd, GWL_STYLE, style & ~WS_SYSMENU);

	if(!bShowDebugConsole) {
		ShowWindow(consoleHwnd, SW_HIDE); //Hide the console window; it just confuses the user;
	}
	if (bEnableControllers)
		controllerThread.run();

	QTimer::singleShot(2000, this, &SteamTargetRenderer::launchApp); // lets steam do its thing

}

SteamTargetRenderer::~SteamTargetRenderer()
{	
	bRunLoop = false;
	renderThread.join();
	if (controllerThread.isRunning())
		controllerThread.stop();
	qpUserWindow->kill();
	delete qpUserWindow;
}

void SteamTargetRenderer::run()
{
	renderThread = std::thread(&SteamTargetRenderer::RunSfWindowLoop, this);
}

void SteamTargetRenderer::RunSfWindowLoop()
{
	if (!bRunLoop)
		return;
	sfWindow.setActive(true);
	DWORD result;
	bool focusSwitchNeeded = true;
	sf::Clock reCheckControllerTimer;

	if (!bDrawOverlay)
	{
		ShowWindow(consoleHwnd, SW_HIDE);
	}

	while (sfWindow.isOpen() && bRunLoop)
	{
		if (bDrawOverlay)
		{
			SetWindowPos(sfWindow.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		sf::Event event;
		while (sfWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				sfWindow.close();
		}

		sfWindow.clear(sf::Color::Transparent);

		if (bDrawDebugEdges)
			drawDebugEdges();

		sfWindow.display();

		//This ensures that we stay in game binding, even if focused application changes! (Why does this work? Well, i dunno... ask Valve...)
		//Only works with a console window
		//Causes trouble as soon as there is more than the consoleWindow and the overlayWindow
		//This is trying to avoid hooking Steam.exe
		if (focusSwitchNeeded)
		{
			focusSwitchNeeded = false;
			SetFocus(consoleHwnd);
			SetForegroundWindow(consoleHwnd);
		}

		//Dirty hack to make the steamoverlay work properly and still keep Apps Controllerconfig when closing overlay.
		//This is trying to avoid hooking Steam.exe
		if (overlayPtr != NULL)
		{
			char overlayOpen = *(char*)overlayPtr;
			if (overlayOpen)
			{
				if (!bNeedFocusSwitch)
				{
					bNeedFocusSwitch = true;

					hwForeGroundWindow = GetForegroundWindow();

					std::cout << "ForegorundWindow HWND: " << hwForeGroundWindow << std::endl;

					SetFocus(consoleHwnd);
					SetForegroundWindow(consoleHwnd);

					SetFocus(sfWindow.getSystemHandle());
					SetForegroundWindow(sfWindow.getSystemHandle());

					SetWindowLong(sfWindow.getSystemHandle(), GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

					SetFocus(consoleHwnd);
					SetForegroundWindow(consoleHwnd);
				}
			}
			else {
				if (bNeedFocusSwitch)
				{

					SetFocus(sfWindow.getSystemHandle());
					SetForegroundWindow(sfWindow.getSystemHandle());

					SetWindowLong(sfWindow.getSystemHandle(), GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

					SetFocus(consoleHwnd);
					SetForegroundWindow(consoleHwnd);

					SetFocus(hwForeGroundWindow);
					SetForegroundWindow(hwForeGroundWindow);

					bNeedFocusSwitch = false;
				}
			}
		}
	}
}

void SteamTargetRenderer::getSteamOverlay()
{
	hmodGameOverlayRenderer = GetModuleHandle(L"Gameoverlayrenderer64.dll");

	if (hmodGameOverlayRenderer != NULL)
	{
		std::cout << "GameOverlayrenderer64.dll found;  Module at: 0x" << hmodGameOverlayRenderer << std::endl;
		overlayPtr = (uint64_t*)(uint64_t(hmodGameOverlayRenderer) + 0x1365e8);
		overlayPtr = (uint64_t*)(*overlayPtr + 0x40);
	}
}


void SteamTargetRenderer::makeSfWindowTransparent(sf::RenderWindow & window)
{
	HWND hwnd = window.getSystemHandle();
	SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP &~WS_CAPTION);
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

	MARGINS margins;
	margins.cxLeftWidth = -1;

	DwmExtendFrameIntoClientArea(hwnd, &margins);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

	window.clear(sf::Color::Transparent);
	window.display();
}

void SteamTargetRenderer::drawDebugEdges()
{
	sfCshape.setPosition(sf::Vector2f(-25, -25));
	sfWindow.draw(sfCshape);
	sfCshape.setPosition(sf::Vector2f(sfWindow.getSize().x + 25, -25));
	sfWindow.draw(sfCshape);
	sfCshape.setPosition(sf::Vector2f(-25, sfWindow.getSize().y));
	sfWindow.draw(sfCshape);
	sfCshape.setPosition(sf::Vector2f(sfWindow.getSize().x, sfWindow.getSize().y));
	sfWindow.draw(sfCshape);

}

void SteamTargetRenderer::openUserWindow()
{
	qpUserWindow = new QProcess(this);
	qpUserWindow->start("SteamTargetUserWindow.exe", QStringList(), QProcess::ReadWrite);
	qpUserWindow->waitForStarted();
	connect(qpUserWindow, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
		this, &SteamTargetRenderer::userWindowFinished);
	connect(qpUserWindow, SIGNAL(readyRead()) , this,SLOT(readChildProcess()));
}

void SteamTargetRenderer::userWindowFinished()
{
	delete qpUserWindow;
	bRunLoop = false;
	renderThread.join();
	if (controllerThread.isRunning())
		controllerThread.stop();
	exit(0);
}

void SteamTargetRenderer::launchApp()
{

	bool launchGame = false;
	QString type = "Win32";
	QString path = "";
	QSettings settings(".\\TargetConfig.ini", QSettings::IniFormat);
	settings.beginGroup("LaunchGame");
	const QStringList childKeys = settings.childKeys();
	for (auto &childkey : childKeys)
	{
		if (childkey == "bLaunchGame")
		{
			launchGame = settings.value(childkey).toBool();
		}
		else if (childkey == "Type") {
			type = settings.value(childkey).toString();
		}
		else if (childkey == "Path") {
			path = settings.value(childkey).toString();
		}
	}
	settings.endGroup();

	if (launchGame)
	{
		QSharedMemory sharedMemInstance("GloSC_GameLauncher");
		if (!sharedMemInstance.create(1024) && sharedMemInstance.error() == QSharedMemory::AlreadyExists)
		{
			QStringList stringList;
			if (type == "Win32")
			{
				stringList << "LaunchWin32Game";
			} else if (type == "UWP") {
				stringList << "LaunchUWPGame";
			}
			stringList << path;

			QBuffer buffer;
			buffer.open(QBuffer::ReadWrite);
			QDataStream out(&buffer);
			out << stringList;
			int size = buffer.size();

			sharedMemInstance.attach();
			char *to = (char*)sharedMemInstance.data();
			const char *from = buffer.data().data();
			memcpy(to, from, qMin(sharedMemInstance.size(), size));
			sharedMemInstance.unlock();
			sharedMemInstance.detach();
		}
	}
}

void SteamTargetRenderer::readChildProcess()
{
	QString message(qpUserWindow->readLine());
	if (message.contains("ResetControllers"))
	{
		if (controllerThread.isRunning())
		{
			controllerThread.stop();
			controllerThread.run();
		}
	} else if (message.contains("ShowConsole")) {
		message.chop(1);
		message.remove("ShowConsole ");
		int showConsole = message.toInt();
		if (showConsole > 0)
		{
			bShowDebugConsole = true;
			ShowWindow(consoleHwnd, SW_SHOW);
			SetFocus(consoleHwnd);
			SetForegroundWindow(consoleHwnd);
		} else {
			bShowDebugConsole = false;
			ShowWindow(consoleHwnd, SW_HIDE);
			SetFocus(consoleHwnd);
			SetForegroundWindow(consoleHwnd);
		}
	} else if (message.contains("ShowOverlay")) {
		message.chop(1);
		message.remove("ShowOverlay ");
		int showOverlay = message.toInt();
		if (showOverlay > 0)
		{
			ShowWindow(sfWindow.getSystemHandle(), SW_SHOW);
			SetFocus(consoleHwnd);
			SetForegroundWindow(consoleHwnd);
		} else {
			ShowWindow(sfWindow.getSystemHandle(), SW_HIDE);
			SetFocus(consoleHwnd);
			SetForegroundWindow(consoleHwnd);
		}
	} else if (message.contains("EnableControllers")) {
		message.chop(1);
		message.remove("EnableControllers ");
		int enableControllers = message.toInt();
		if (enableControllers > 0)
		{
			bEnableControllers = true;
			if (!controllerThread.isRunning())
				controllerThread.run();
		} else {
			bEnableControllers = false;
			if (controllerThread.isRunning())
				controllerThread.stop();
		}
	}
}
