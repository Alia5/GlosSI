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

	//SetDebugPrivilege();

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
	sharedMemInstance.unlock();

	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(isAboutToBeKilled()));
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(checkSharedMem()));

	updateTimer.setInterval(1000);
	updateTimer.start();
}

void GloSC_GameLauncher::isAboutToBeKilled()
{
	sharedMemInstance.detach();
}

void GloSC_GameLauncher::checkSharedMem()
{
	QBuffer buffer;
	QDataStream in(&buffer);
	QStringList stringList;

	sharedMemInstance.lock();

	if (pid != NULL)
	{
		memset(sharedMemInstance.data(), NULL, 1024);
		if (!IsProcessRunning(pid))
		{
			pid = NULL;
			stringListFromShared = stringList;
			stringList << "LaunchedProcessFinished";
			buffer.open(QBuffer::ReadWrite);
			QDataStream out(&buffer);
			out << stringList;
			int size = buffer.size();
			char *to = (char*)sharedMemInstance.data();
			const char *from = buffer.data().data();
			memcpy(to, from, qMin(sharedMemInstance.size(), size));
			sharedMemInstance.unlock();
		}
	} else {
		buffer.setData((char*)sharedMemInstance.constData(), sharedMemInstance.size());
		buffer.open(QBuffer::ReadOnly);
		in >> stringList;
		stringListFromShared = stringList;
		memset(sharedMemInstance.data(), NULL, 1024);
		sharedMemInstance.unlock();

		launchGameIfRequired();
	}


}

void GloSC_GameLauncher::launchGameIfRequired()
{
	if (stringListFromShared.size() > 1)
	{
		if (stringListFromShared.at(0) == "LaunchWin32Game")
		{
			QProcess app;
			if (stringListFromShared.at(1).contains("\\"))
			{
				app.startDetached(stringListFromShared.at(1), QStringList(), stringListFromShared.at(1).mid(0, stringListFromShared.at(1).lastIndexOf("\\")), &pid);
			}
			else
			{
				app.startDetached(stringListFromShared.at(1), QStringList(), stringListFromShared.at(1).mid(0, stringListFromShared.at(1).lastIndexOf("/")), &pid);

			}
		} else if (stringListFromShared.at(0) == "LaunchUWPGame") {
			DWORD pid = 0;
			HRESULT hr = CoInitialize(nullptr);
			std::wstring appUMId = stringListFromShared.at(1).toStdWString();
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

