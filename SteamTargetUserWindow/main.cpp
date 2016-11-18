#include "SteamTargetUserWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SteamTargetUserWindow w;
	w.setAttribute(Qt::WA_ShowWithoutActivating);
    w.show();
    return a.exec();
}
