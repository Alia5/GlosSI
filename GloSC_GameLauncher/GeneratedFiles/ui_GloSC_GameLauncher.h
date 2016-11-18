/********************************************************************************
** Form generated from reading UI file 'GloSC_GameLauncher.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GLOSC_GAMELAUNCHER_H
#define UI_GLOSC_GAMELAUNCHER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GloSC_GameLauncherClass
{
public:
    QWidget *centralWidget;

    void setupUi(QMainWindow *GloSC_GameLauncherClass)
    {
        if (GloSC_GameLauncherClass->objectName().isEmpty())
            GloSC_GameLauncherClass->setObjectName(QStringLiteral("GloSC_GameLauncherClass"));
        GloSC_GameLauncherClass->setEnabled(false);
        GloSC_GameLauncherClass->resize(600, 400);
        centralWidget = new QWidget(GloSC_GameLauncherClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        GloSC_GameLauncherClass->setCentralWidget(centralWidget);

        retranslateUi(GloSC_GameLauncherClass);

        QMetaObject::connectSlotsByName(GloSC_GameLauncherClass);
    } // setupUi

    void retranslateUi(QMainWindow *GloSC_GameLauncherClass)
    {
        GloSC_GameLauncherClass->setWindowTitle(QApplication::translate("GloSC_GameLauncherClass", "GloSC_GameLauncher", 0));
    } // retranslateUi

};

namespace Ui {
    class GloSC_GameLauncherClass: public Ui_GloSC_GameLauncherClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLOSC_GAMELAUNCHER_H
