/********************************************************************************
** Form generated from reading UI file 'GloSC.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GLOSC_H
#define UI_GLOSC_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GloSCClass
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QListWidget *lwInstances;
    QGroupBox *groupBox_2;
    QLineEdit *lePath;
    QPushButton *pbUWP;
    QPushButton *pbSearchPath;
    QCheckBox *cbDebug;
    QCheckBox *cbOverlay;
    QCheckBox *cbControllers;
    QCheckBox *cbVsync;
    QSpinBox *sbRefresh;
    QCheckBox *cbLaunchGame;
    QLabel *label;
    QLabel *label_2;
    QPushButton *pbSave;
    QPushButton *pbAddToSteam;
    QLabel *label_3;
    QLineEdit *leName;
    QPushButton *pbDelete;

    void setupUi(QMainWindow *GloSCClass)
    {
        if (GloSCClass->objectName().isEmpty())
            GloSCClass->setObjectName(QStringLiteral("GloSCClass"));
        GloSCClass->resize(711, 386);
        GloSCClass->setMinimumSize(QSize(711, 386));
        GloSCClass->setMaximumSize(QSize(711, 386));
        centralWidget = new QWidget(GloSCClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 281, 371));
        lwInstances = new QListWidget(groupBox);
        lwInstances->setObjectName(QStringLiteral("lwInstances"));
        lwInstances->setGeometry(QRect(10, 21, 261, 341));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(309, 10, 391, 371));
        lePath = new QLineEdit(groupBox_2);
        lePath->setObjectName(QStringLiteral("lePath"));
        lePath->setGeometry(QRect(40, 281, 261, 20));
        pbUWP = new QPushButton(groupBox_2);
        pbUWP->setObjectName(QStringLiteral("pbUWP"));
        pbUWP->setGeometry(QRect(344, 280, 41, 23));
        pbSearchPath = new QPushButton(groupBox_2);
        pbSearchPath->setObjectName(QStringLiteral("pbSearchPath"));
        pbSearchPath->setGeometry(QRect(300, 280, 41, 23));
        cbDebug = new QCheckBox(groupBox_2);
        cbDebug->setObjectName(QStringLiteral("cbDebug"));
        cbDebug->setGeometry(QRect(10, 80, 121, 17));
        cbOverlay = new QCheckBox(groupBox_2);
        cbOverlay->setObjectName(QStringLiteral("cbOverlay"));
        cbOverlay->setGeometry(QRect(10, 110, 111, 17));
        cbOverlay->setChecked(true);
        cbControllers = new QCheckBox(groupBox_2);
        cbControllers->setObjectName(QStringLiteral("cbControllers"));
        cbControllers->setGeometry(QRect(10, 140, 151, 17));
        cbControllers->setChecked(true);
        cbVsync = new QCheckBox(groupBox_2);
        cbVsync->setObjectName(QStringLiteral("cbVsync"));
        cbVsync->setGeometry(QRect(10, 170, 131, 17));
        cbVsync->setChecked(true);
        sbRefresh = new QSpinBox(groupBox_2);
        sbRefresh->setObjectName(QStringLiteral("sbRefresh"));
        sbRefresh->setGeometry(QRect(90, 210, 42, 22));
        sbRefresh->setMinimum(30);
        sbRefresh->setMaximum(240);
        sbRefresh->setValue(60);
        cbLaunchGame = new QCheckBox(groupBox_2);
        cbLaunchGame->setObjectName(QStringLiteral("cbLaunchGame"));
        cbLaunchGame->setGeometry(QRect(10, 260, 91, 17));
        label = new QLabel(groupBox_2);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 281, 31, 20));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 215, 71, 16));
        pbSave = new QPushButton(groupBox_2);
        pbSave->setObjectName(QStringLiteral("pbSave"));
        pbSave->setGeometry(QRect(10, 330, 151, 31));
        pbAddToSteam = new QPushButton(groupBox_2);
        pbAddToSteam->setObjectName(QStringLiteral("pbAddToSteam"));
        pbAddToSteam->setEnabled(false);
        pbAddToSteam->setGeometry(QRect(300, 330, 81, 31));
        pbAddToSteam->setAutoDefault(false);
        pbAddToSteam->setFlat(false);
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 33, 47, 13));
        leName = new QLineEdit(groupBox_2);
        leName->setObjectName(QStringLiteral("leName"));
        leName->setGeometry(QRect(50, 30, 331, 20));
        pbDelete = new QPushButton(groupBox_2);
        pbDelete->setObjectName(QStringLiteral("pbDelete"));
        pbDelete->setGeometry(QRect(170, 330, 121, 31));
        GloSCClass->setCentralWidget(centralWidget);

        retranslateUi(GloSCClass);

        pbAddToSteam->setDefault(false);


        QMetaObject::connectSlotsByName(GloSCClass);
    } // setupUi

    void retranslateUi(QMainWindow *GloSCClass)
    {
        GloSCClass->setWindowTitle(QApplication::translate("GloSCClass", "GloSC", 0));
        groupBox->setTitle(QApplication::translate("GloSCClass", "Instances", 0));
        groupBox_2->setTitle(QApplication::translate("GloSCClass", "Config", 0));
        pbUWP->setText(QApplication::translate("GloSCClass", "UWP", 0));
        pbSearchPath->setText(QApplication::translate("GloSCClass", "...", 0));
        cbDebug->setText(QApplication::translate("GloSCClass", "Show debug console", 0));
        cbOverlay->setText(QApplication::translate("GloSCClass", "Enable overlay", 0));
        cbControllers->setText(QApplication::translate("GloSCClass", "Enable virtual controllers", 0));
        cbVsync->setText(QApplication::translate("GloSCClass", "VSync", 0));
        cbLaunchGame->setText(QApplication::translate("GloSCClass", "Launch game", 0));
        label->setText(QApplication::translate("GloSCClass", "Path: ", 0));
        label_2->setText(QApplication::translate("GloSCClass", "Refresh Rate: ", 0));
        pbSave->setText(QApplication::translate("GloSCClass", "Save / Create", 0));
        pbAddToSteam->setText(QApplication::translate("GloSCClass", "Add to Steam", 0));
        label_3->setText(QApplication::translate("GloSCClass", "Name: ", 0));
        pbDelete->setText(QApplication::translate("GloSCClass", "Delete", 0));
    } // retranslateUi

};

namespace Ui {
    class GloSCClass: public Ui_GloSCClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLOSC_H
