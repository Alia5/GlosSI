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
#include "SteamTargetRenderer.h"

std::atomic<bool> SteamTargetRenderer::overlayOpen = false;
HHOOK SteamTargetRenderer::hook = nullptr;

SteamTargetRenderer::SteamTargetRenderer(int& argc, char** argv) : QApplication(argc, argv)
{
	getSteamOverlay();
	loadLogo();

	QSettings settings(".\\TargetConfig.ini", QSettings::IniFormat);
	settings.beginGroup("BaseConf");
	const QStringList childKeys = settings.childKeys();
	for (auto &childkey : childKeys)
	{
		if (childkey == "bDrawDebugEdges")
		{
			bDrawDebugEdges = settings.value(childkey).toBool();
		} else if (childkey == "bEnableOverlay") {
			bDrawOverlay = settings.value(childkey).toBool();
		} else if (childkey == "bEnableControllers") {
			bEnableControllers = settings.value(childkey).toBool();
		}else if (childkey == "bHookSteam") {
			bHookSteam = settings.value(childkey).toBool();
		}
		else if (childkey == "bUseDesktopConfig") {
			bUseDesktopConfig = settings.value(childkey).toBool();
		}
	}
	settings.endGroup();

#ifndef NDEBUG
	bDrawDebugEdges = true;
#endif // NDEBUG
	sfCshape = sf::CircleShape(100.f);
	sfCshape.setFillColor(sf::Color(128, 128, 128, 128));
	sfCshape.setOrigin(sf::Vector2f(100, 100));
	sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	sfWindow.create(sf::VideoMode(mode.width-16, mode.height-32), "GloSC_OverlayWindow"); //Window is too large ; always 16 and 32 pixels?  - sf::Style::None breaks transparency!
	sfWindow.setFramerateLimit(iRefreshRate);
	sfWindow.setPosition(sf::Vector2i(0, 0));
	makeSfWindowTransparent(sfWindow);

	sfWindow.setActive(false);
	consoleHwnd = GetConsoleWindow(); //We need a console for a dirty hack to make sure we stay in game bindings
									  //QT Windows cause trouble with the overlay, so we cannot use them

	//ShowWindow(consoleHwnd, SW_HIDE);

	if (bEnableControllers)
		controllerThread.run();

	QTimer::singleShot(2000, this, &SteamTargetRenderer::launchApp); // lets steam do its thing

	if (hmodGameOverlayRenderer != nullptr)
	{
		//Hook MessageQueue to detect if overlay gets opened / closed
		//Steam Posts a Message with 0x14FA / 0x14F7 when the overlay gets opened / closed
		hook = SetWindowsHookEx(WH_GETMESSAGE, HookCallback, nullptr, GetCurrentThreadId());
	}


	if (bUseDesktopConfig)
	{
		bHookSteam = false;
		QTimer::singleShot(1000, this, []()
		{
			HWND taskbar = FindWindow(L"Shell_TrayWnd", nullptr);
			SetFocus(taskbar);
			SetForegroundWindow(taskbar);
		});
	}

}

SteamTargetRenderer::~SteamTargetRenderer()
{	

	if (hmodGameOverlayRenderer != nullptr)
	{
		UnhookWindowsHookEx(hook);
	}

	renderThread.join();
	if (controllerThread.isRunning())
		controllerThread.stop();
}

void SteamTargetRenderer::run()
{
	renderThread = std::thread(&SteamTargetRenderer::RunSfWindowLoop, this);
}

void SteamTargetRenderer::stop()
{
	bRunLoop = false;
	QApplication::exit(0);
}


void SteamTargetRenderer::RunSfWindowLoop()
{
	if (!bRunLoop)
		return;
	sfWindow.setActive(true);

	bool focusSwitchNeeded = true;

	if (bDrawOverlay)
	{
		SetWindowPos(sfWindow.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_ASYNCWINDOWPOS);
	} else {
		ShowWindow(consoleHwnd, SW_SHOW);
	}
	

	while (sfWindow.isOpen() && bRunLoop)
	{

		sf::Event event;
		while (sfWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				sfWindow.close();
		}

		sfWindow.clear(sf::Color::Transparent);

		if (bDrawDebugEdges)
			drawDebugEdges();


		//This ensures that we stay in game binding, even if focused application changes! (Why does this work? Well, i dunno... ask Valve...)
		//Only works with a console window
		//Causes trouble as soon as there is more than the consoleWindow and the overlayWindow
		//This is trying to avoid hooking Steam.exe
		//----
		//alternatively / additionaly, we can just hook steam and make our lives so much easier
		//we inject and hook here to spare IPC and let the dll grab the steam appID of the launched process when the config switches (config switches w/ focus)
		if (focusSwitchNeeded)
		{

			if (bHookSteam)
				hookBindings(); //cleanup - unhooking / unloading of dll is managed by the GloSC gamelauncher rather than here

			focusSwitchNeeded = false;
			if (!bUseDesktopConfig)
			{
				SetFocus(consoleHwnd);
				sf::Clock clock;
				while (!SetForegroundWindow(consoleHwnd) && clock.getElapsedTime().asMilliseconds() < 1000) //try to forcefully set foreground window
				{
					Sleep(1);
				}
			}
		}

		//Dirty hack to make the steamoverlay work properly and still keep Apps Controllerconfig when closing overlay.
		//even if hooking steam, this ensures the overlay stays working
		if (hmodGameOverlayRenderer != NULL)
		{
			if (overlayOpen)
			{
				if (!bNeedFocusSwitch)
				{
					bNeedFocusSwitch = true;

					hwForeGroundWindow = GetForegroundWindow();

					std::cout << "Saving current ForegorundWindow HWND: " << hwForeGroundWindow << std::endl;
					std::cout << "Activating OverlayWindow" << std::endl;

					SetWindowLong(sfWindow.getSystemHandle(), GWL_EXSTYLE, WS_EX_LAYERED); //make overlay window clickable

					//Actually activate the overlaywindow
					SetFocus(sfWindow.getSystemHandle());

					if (!bUseDesktopConfig)
					{
						//by activating the consolewindow **and bringing it to the foreground** we can trick steam so the controller stays in game bindings
						SetFocus(consoleHwnd);
						sf::Clock clock;
						while (!SetForegroundWindow(consoleHwnd) && clock.getElapsedTime().asMilliseconds() < 1000) //try to forcefully set foreground window
						{
							Sleep(1);
						}
					}

					//Move the mouse cursor inside the overlaywindow
					//this is neccessary because steam doesn't want to switch to big picture bindings if mouse isn't inside
					SetCursorPos(16, 16);
				}
				sfWindow.draw(backgroundSprite);
			} else {
				if (bNeedFocusSwitch)
				{
					std::cout << "Deactivating OverlayWindow" << std::endl;

					//make overlaywindow clickthrough - WS_EX_TRANSPARENT - again
					SetWindowLong(sfWindow.getSystemHandle(), GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);

					std::cout << "Switching to previously focused window" << std::endl;

					//switch back the the previosly focused window
					SetFocus(hwForeGroundWindow);
					sf::Clock clock;
					while (!SetForegroundWindow(hwForeGroundWindow) && clock.getElapsedTime().asMilliseconds() < 1000) //try to forcefully set foreground window
					{
						Sleep(1);
					}
					bNeedFocusSwitch = false;
				}
			}
		}
		sfWindow.display();
	}
	stop();
}

void SteamTargetRenderer::getSteamOverlay()
{
	hmodGameOverlayRenderer = GetModuleHandle(overlayModuleName);

	if (hmodGameOverlayRenderer != nullptr)
	{
		std::cout << overlayModuleName << " found;  Module at: 0x" << hmodGameOverlayRenderer << std::endl;
	}
}


void SteamTargetRenderer::makeSfWindowTransparent(sf::RenderWindow & window)
{
	HWND hwnd = window.getSystemHandle();
	SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP &~WS_CAPTION);
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);

	MARGINS margins;
	margins.cxLeftWidth = -1;

	DwmExtendFrameIntoClientArea(hwnd, &margins);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);


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

void SteamTargetRenderer::hookBindings()
{
	std::cout << "Hooking Steam..." << std::endl;

	QString dir = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
	dir = dir.mid(0, dir.lastIndexOf("\\"));

	QProcess proc;
	proc.setNativeArguments(" --inject ");
	proc.setWorkingDirectory(dir);
	proc.start("..\\Injector.exe", QIODevice::ReadOnly);
	proc.waitForStarted();
	proc.waitForFinished();

	if (QString::fromStdString(proc.readAll().toStdString()).contains("Inject success!")) //if we have injected (and patched the function)
	{
		std::cout << "Successfully hooked Steam!" << std::endl;

		//tell the GloSC_GameLauncher that we have hooked steam
		//it will deal with checking if the target is still alive and unload the dll / unhook then
		// - ensures unloading / unhooking even if this process crashes or gets unexpectedly killed
		QSharedMemory sharedMemInstance("GloSC_GameLauncher");
		if (!sharedMemInstance.create(1024) && sharedMemInstance.error() == QSharedMemory::AlreadyExists)
		{
			QBuffer buffer;
			QDataStream dataStream(&buffer);
			QStringList stringList;

			sharedMemInstance.attach();
			sharedMemInstance.lock();

			buffer.setData(static_cast<const char*>(sharedMemInstance.constData()), sharedMemInstance.size());
			buffer.open(QBuffer::ReadOnly);
			dataStream >> stringList;
			buffer.close();

			int i = stringList.indexOf(IsSteamHooked) + 1;
			stringList.replace(i, "1");


			buffer.open(QBuffer::ReadWrite);
			QDataStream out(&buffer);
			out << stringList;
			int size = buffer.size();
			char *to = static_cast<char*>(sharedMemInstance.data());
			const char *from = buffer.data().data();
			memcpy(to, from, qMin(sharedMemInstance.size(), size));
			buffer.close();

			sharedMemInstance.unlock();
			sharedMemInstance.detach();
		}
	} else {
		std::cout << "Hooking Steam failed!" << std::endl;
		
		MessageBoxW(NULL, L"Hooking Steam failed!", L"GloSC-SteamTarget", MB_OK);
	}
}

void SteamTargetRenderer::loadLogo()
{
	HRSRC rsrcData = FindResource(NULL, L"ICOPNG", RT_RCDATA);
	DWORD rsrcDataSize = SizeofResource(NULL, rsrcData);
	HGLOBAL grsrcData = LoadResource(NULL, rsrcData);
	LPVOID firstByte = LockResource(grsrcData);
	spriteTexture = std::make_unique<sf::Texture>();
	spriteTexture->loadFromMemory(firstByte, rsrcDataSize);
	backgroundSprite.setTexture(*spriteTexture);
	backgroundSprite.setOrigin(sf::Vector2f(spriteTexture->getSize().x / 2.f, spriteTexture->getSize().y / 2));
	sf::VideoMode winSize = sf::VideoMode::getDesktopMode();
	backgroundSprite.setPosition(sf::Vector2f(winSize.width / 2.f, winSize.height / 2.f));
}

LRESULT WINAPI SteamTargetRenderer::HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		PMSG msg = reinterpret_cast<PMSG>(lParam);
		if (msg->message == 0x14FA) //Posted when the overlay gets opened
		{
			overlayOpen = true;
			std::cout << "Overlay Opened!\n";
		}
		else if (msg->message == 0x14F7 || msg->message == 512)
		{
			overlayOpen = false;
			std::cout << "Overlay closed!\n";
		}
	}
	return CallNextHookEx(hook, nCode, wParam, lParam);
}

void SteamTargetRenderer::launchApp()
{
	bool launchGame = false;
	bool closeWhenDone = false;
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
		else if (childkey == "bCloseWhenDone") {
			closeWhenDone = settings.value("bCloseWhenDone").toBool();
		}
	}
	settings.endGroup();

	if (launchGame)
	{
		QSharedMemory sharedMemInstance("GloSC_GameLauncher");
		if (!sharedMemInstance.create(1024) && sharedMemInstance.error() == QSharedMemory::AlreadyExists)
		{
			QBuffer buffer;
			QDataStream dataStream(&buffer);
			QStringList stringList;

			sharedMemInstance.attach();
			sharedMemInstance.lock();

			buffer.setData(static_cast<const char*>(sharedMemInstance.constData()), sharedMemInstance.size());
			buffer.open(QBuffer::ReadOnly);
			dataStream >> stringList;
			buffer.close();



			int lgt_index = stringList.indexOf(LaunchGame);
			stringList.replace(lgt_index + 1, type);
			stringList.replace(lgt_index + 2, path);



			buffer.open(QBuffer::ReadWrite);
			QDataStream out(&buffer);
			out << stringList;
			int size = buffer.size();
			char *to = static_cast<char*>(sharedMemInstance.data());
			const char *from = buffer.data().data();
			memcpy(to, from, qMin(sharedMemInstance.size(), size));
			buffer.close();

			sharedMemInstance.unlock();
			sharedMemInstance.detach();

			if (closeWhenDone)
			{
				updateTimer.setInterval(1111);
				connect(&updateTimer, SIGNAL(timeout()), this, SLOT(checkSharedMem()));
				updateTimer.start();
			}
		}
	}
}

void SteamTargetRenderer::checkSharedMem()
{
	QSharedMemory sharedMemInstance("GloSC_GameLauncher");
	if (!sharedMemInstance.create(1024) && sharedMemInstance.error() == QSharedMemory::AlreadyExists)
	{
		QBuffer buffer;
		QDataStream in(&buffer);
		QStringList stringList;

		sharedMemInstance.attach();
		sharedMemInstance.lock();
		buffer.setData(static_cast<const char*>(sharedMemInstance.constData()), sharedMemInstance.size());
		buffer.open(QBuffer::ReadOnly);
		in >> stringList;
		buffer.close();
		sharedMemInstance.unlock();
		sharedMemInstance.detach();

		int close_index = stringList.indexOf(LaunchedProcessFinished)+1;

		if (close_index > 0 && stringList.at(close_index).toInt() == 1)
		{
			bRunLoop = false;
			renderThread.join();
			if (controllerThread.isRunning())
				controllerThread.stop();
			exit(0);
		}
	}


}

