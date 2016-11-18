#include "GloSC.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GloSC w;
    w.show();
    return a.exec();
}
