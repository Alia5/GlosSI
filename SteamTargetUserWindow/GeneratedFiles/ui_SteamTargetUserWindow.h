/********************************************************************************
** Form generated from reading UI file 'SteamTargetUserWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STEAMTARGETUSERWINDOW_H
#define UI_STEAMTARGETUSERWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SteamTargetUserWindowClass
{
public:
    QWidget *centralWidget;
    QCheckBox *checkBox_showOverlay;
    QCheckBox *checkBox_enableControllers;
    QPushButton *pushButton_resetControllers;
    QCheckBox *checkBox_showDebugConsole;

    void setupUi(QMainWindow *SteamTargetUserWindowClass)
    {
        if (SteamTargetUserWindowClass->objectName().isEmpty())
            SteamTargetUserWindowClass->setObjectName(QStringLiteral("SteamTargetUserWindowClass"));
        SteamTargetUserWindowClass->resize(282, 137);
        SteamTargetUserWindowClass->setMinimumSize(QSize(282, 137));
        SteamTargetUserWindowClass->setMaximumSize(QSize(282, 137));
        centralWidget = new QWidget(SteamTargetUserWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        checkBox_showOverlay = new QCheckBox(centralWidget);
        checkBox_showOverlay->setObjectName(QStringLiteral("checkBox_showOverlay"));
        checkBox_showOverlay->setGeometry(QRect(20, 10, 121, 17));
        checkBox_enableControllers = new QCheckBox(centralWidget);
        checkBox_enableControllers->setObjectName(QStringLiteral("checkBox_enableControllers"));
        checkBox_enableControllers->setGeometry(QRect(20, 40, 141, 17));
        pushButton_resetControllers = new QPushButton(centralWidget);
        pushButton_resetControllers->setObjectName(QStringLiteral("pushButton_resetControllers"));
        pushButton_resetControllers->setGeometry(QRect(20, 100, 241, 23));
        checkBox_showDebugConsole = new QCheckBox(centralWidget);
        checkBox_showDebugConsole->setObjectName(QStringLiteral("checkBox_showDebugConsole"));
        checkBox_showDebugConsole->setGeometry(QRect(20, 70, 141, 17));
        SteamTargetUserWindowClass->setCentralWidget(centralWidget);

        retranslateUi(SteamTargetUserWindowClass);

        QMetaObject::connectSlotsByName(SteamTargetUserWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *SteamTargetUserWindowClass)
    {
        SteamTargetUserWindowClass->setWindowTitle(QApplication::translate("SteamTargetUserWindowClass", "GloSC", 0));
        checkBox_showOverlay->setText(QApplication::translate("SteamTargetUserWindowClass", "Enable Overlay", 0));
        checkBox_enableControllers->setText(QApplication::translate("SteamTargetUserWindowClass", "Enable Virtual Controllers", 0));
        pushButton_resetControllers->setText(QApplication::translate("SteamTargetUserWindowClass", "Reset Controllers / Redetect real controllers", 0));
        checkBox_showDebugConsole->setText(QApplication::translate("SteamTargetUserWindowClass", "Show debug console", 0));
    } // retranslateUi

};

namespace Ui {
    class SteamTargetUserWindowClass: public Ui_SteamTargetUserWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STEAMTARGETUSERWINDOW_H
