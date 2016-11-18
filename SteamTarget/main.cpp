#include "SteamTargetRenderer.h"
#include <QtCore/QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	SteamTargetRenderer renderer;
	renderer.run();
	return a.exec();
}

