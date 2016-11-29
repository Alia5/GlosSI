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
#include "GloSC_GameLauncher.h"

GloSC_GameLauncher::GloSC_GameLauncher(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QTimer::singleShot(0, this, SLOT(hide()));

	sharedMemInstance.setKey("GloSC_GameLauncher");
	sharedMemInstance.create(1024);

	if (!sharedMemInstance.create(1024) && sharedMemInstance.error() == QSharedMemory::AlreadyExists)
	{
		exit(1);
	}

	sharedMemInstance.attach();
	sharedMemInstance.lock();
	memset(sharedMemInstance.data(), NULL, 1024);

	QBuffer buffer;
	buffer.open(QBuffer::ReadWrite);
	QDataStream out(&buffer);
	out << defaultSharedMemData;
	int size = buffer.size();
	char *to = (char*)sharedMemInstance.data();
	const char *from = buffer.data().data();
	memcpy(to, from, qMin(sharedMemInstance.size(), size));

	sharedMemInstance.unlock();


	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(isAboutToBeKilled()));
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(checkSharedMem()));

	updateTimer.setInterval(333);
	updateTimer.start();
}

void GloSC_GameLauncher::isAboutToBeKilled()
{
	sharedMemInstance.detach();
}

void GloSC_GameLauncher::checkSharedMem()
{
	QBuffer buffer;
	QDataStream dataStream(&buffer);
	QStringList stringList;

	sharedMemInstance.lock();

	buffer.setData((char*)sharedMemInstance.constData(), sharedMemInstance.size());
	buffer.open(QBuffer::ReadOnly);
	dataStream >> stringList;
	buffer.close();
	int i = stringList.indexOf(LaunchGame);
	if (i > -1)
	{
		if (stringList.at(i + 1) != "" && stringList.at(i + 2) != "")
		{
			launchGame(stringList.at(i + 1), stringList.at(i + 2));
			stringList = defaultSharedMemData;
		}
	}

	i = stringList.indexOf(IsSteamHooked);
	if (i > -1)
	{
		bHookedSteam = true;
		if (stringList.at(i + 1).toInt() > -1)
		{
			stringList.replace(i + 1, "-1");
		}
	}

	if (pid != NULL)
	{
		if (!IsProcessRunning(pid))
		{
			pid = NULL;
			int i = stringList.indexOf(LaunchedProcessFinished) + 1;
			stringList.replace(i, "1");
		}
	}

	buffer.open(QBuffer::ReadWrite);
	QDataStream out(&buffer);
	out << stringList;
	int size = buffer.size();
	char *to = (char*)sharedMemInstance.data();
	const char *from = buffer.data().data();
	memcpy(to, from, qMin(sharedMemInstance.size(), size));
	buffer.close();

	sharedMemInstance.unlock();

	if (FindWindow(NULL, L"GloSC_OverlayWindow") == NULL)
	{
		unhookBindings();
		bHookedSteam = false;
	}
}

void GloSC_GameLauncher::launchGame(QString type, QString path)
{

		if (type == LGT_Win32)
		{
			QProcess app;
			if (path.contains("\\"))
			{
				app.startDetached(path, QStringList(), path.mid(0, path.lastIndexOf("\\")), &pid);
			}
			else
			{
				app.startDetached(path, QStringList(), path.mid(0, path.lastIndexOf("/")), &pid);

			}
		} else if (type == LGT_UWP) {
			DWORD pid = 0;
			HRESULT hr = CoInitialize(nullptr);
			std::wstring appUMId = path.toStdWString();
			if (SUCCEEDED(hr)) {
				HRESULT result = LaunchUWPApp(appUMId.c_str(), &pid);
				if (SUCCEEDED(result))
				{
					this->pid = pid;
				
				}
			}
			CoUninitialize();

		}
}

HRESULT GloSC_GameLauncher::LaunchUWPApp(LPCWSTR packageFullName, PDWORD pdwProcessId)
{

	CComPtr<IApplicationActivationManager> spAppActivationManager;
	HRESULT result = E_INVALIDARG;
	// Initialize IApplicationActivationManager
	result = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_LOCAL_SERVER, IID_IApplicationActivationManager, (LPVOID*)&spAppActivationManager);

	if (!SUCCEEDED(result))
		return result;

	//This call causes troubles; especially with our always in foreground overlay-window
	/*
	// This call ensures that the app is launched as the foreground window
	result = CoAllowSetForegroundWindow(spAppActivationManager, NULL);


	if (!SUCCEEDED(result))
		return result;
		*/

	// Launch the app
	result = spAppActivationManager->ActivateApplication(packageFullName, NULL, AO_NONE, pdwProcessId);

	return result;
}

bool GloSC_GameLauncher::IsProcessRunning(DWORD pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 1);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}

void GloSC_GameLauncher::unhookBindings()
{
	QProcess proc;
	proc.setNativeArguments(" --eject ");
	proc.start("Injector.exe", QIODevice::ReadOnly);
	proc.waitForFinished();
	bHookedSteam = false;
}



