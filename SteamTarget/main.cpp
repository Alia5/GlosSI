#include "SteamTarget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SteamTarget w;
    w.show();
    return a.exec();
}
