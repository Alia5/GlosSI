#include "GloSC_GameLauncher.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GloSC_GameLauncher w;
    w.show();

	QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(isAboutToBeKilled()));

    return a.exec();
}
