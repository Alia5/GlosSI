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

	//connect(&qApp, SIGNAL(aboutToQuit()), this SLOT(isAboutToBeKilled()));
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
	buffer.setData((char*)sharedMemInstance.constData(), sharedMemInstance.size());
	buffer.open(QBuffer::ReadOnly);
	in >> stringList;
	stringListFromShared = stringList;
	memset(sharedMemInstance.data(), NULL, 1024);
	sharedMemInstance.unlock();

	launchGameIfRequired();

}

void GloSC_GameLauncher::launchGameIfRequired()
{
	if (stringListFromShared.size() > 1)
	{
		if (stringListFromShared.at(0) == "LaunchWin32Game")
		{
			//At this point, we start detached
			//Listening while an app is alive and the reporting back? Naah, too much work.
			QProcess app;
			if (stringListFromShared.at(1).contains("\\"))
			{

				app.startDetached(stringListFromShared.at(1), QStringList(), stringListFromShared.at(1).mid(0, stringListFromShared.at(1).lastIndexOf("\\")));
			}
			else
			{
				app.startDetached(stringListFromShared.at(1), QStringList(), stringListFromShared.at(1).mid(0, stringListFromShared.at(1).lastIndexOf("/")));
			}
		} else if (stringListFromShared.at(0) == "LaunchUWPGame") {
			//UWP Games cannot be opened twice. No further checking at this point
			DWORD pid = 0;
			HRESULT hr = CoInitialize(nullptr);
			std::wstring appUMId = stringListFromShared.at(1).toStdWString();
			if (SUCCEEDED(hr)) {
				HRESULT result = LaunchUWPApp(appUMId.c_str(), &pid);
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
	//I was way to lazy to patch the Steamtarget... Ouh well...

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

